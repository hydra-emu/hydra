#include "mainwindow.hxx"
#include "aboutwindow.hxx"
#include "downloaderwindow.hxx"
#include "input.hxx"
#include "qthelper.hxx"
#include "scripteditor.hxx"
#include "settingswindow.hxx"
#include "shadereditor.hxx"
#include "terminalwindow.hxx"
#include <compatibility.hxx>
#include <csignal>
#include <error_factory.hxx>
#include <fmt/format.h>
#include <fstream>
#include <hydra/core.hxx>
#include <iostream>
#include <json.hpp>
#include <log.h>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QSurfaceFormat>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QTimer>
#include <settings.hxx>
#ifdef HYDRA_USE_LUA
#include <sol/sol.hpp>
#endif
#include <stb_image_write.h>
#include <str_hash.hxx>
#include <update.hxx>
// TODO: remove this
#define OSSL_DEPRECATEDIN_3_0
#include <openssl/md5.h>

enum class EmulatorState
{
    NOTRUNNING,
    RUNNING,
    STOP,
    STOPPED,
};
std::atomic<EmulatorState> emulator_thread_state;

MainWindow* main_window = nullptr;

void hungry_for_more(ma_device* device, void* out, const void*, ma_uint32 frames)
{
    MainWindow* window = static_cast<MainWindow*>(device->pUserData);
    std::unique_lock<std::mutex> lock(window->audio_mutex_);
    if (window->queued_audio_.empty())
    {
        return;
    }
    frames = std::min(frames * 2, static_cast<ma_uint32>(window->queued_audio_.size()));
    std::memcpy(out, window->queued_audio_.data(), frames * sizeof(int16_t));
    window->queued_audio_.erase(window->queued_audio_.begin(),
                                window->queued_audio_.begin() + frames);
}

void emulator_signal_handler(int signal)
{
    std::lock_guard<std::mutex> lock(main_window->emulator_mutex_);
    printf("Received signal in emulator: %d, stopping...\n", signal);
    main_window->stop_emulator();
}

void initialize_signals()
{
    signal(SIGSEGV, emulator_signal_handler);
    signal(SIGILL, emulator_signal_handler);
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    main_window = this;

    emulator_thread_state = EmulatorState::NOTRUNNING;

    QWidget* widget = new QWidget;
    setCentralWidget(widget);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setContentsMargins(5, 5, 5, 5);
    screen_ = new ScreenWidget(widget);
    screen_->SetMouseMoveCallback([this](QMouseEvent* event) { on_mouse_move(event); });
    screen_->SetMouseClickCallback([this](QMouseEvent* event) { on_mouse_click(event); });
    screen_->SetMouseReleaseCallback([this](QMouseEvent* event) { on_mouse_release(event); });
    screen_->setMouseTracking(true);
    layout->addWidget(screen_, Qt::AlignCenter);
    widget->setLayout(layout);
    create_actions();
    create_menus();

    QString message = tr("Welcome to hydra!");
    statusBar()->showMessage(message);
    setMinimumSize(160, 160);
    resize(640, 480);
    setWindowTitle("hydra");
    setWindowIcon(QIcon(":/images/hydra.png"));

    emulator_timer_ = new QTimer(this);
    connect(emulator_timer_, SIGNAL(timeout()), this, SLOT(emulator_frame()));

    initialize_audio();
    enable_emulation_actions(false);
    screen_->show();

    widget->setStyleSheet(R"(
        background-repeat: no-repeat;
        background-position: center;
        background-image: url(:/images/hydra.png);
    )");

    QFuture<hydra::Updater::UpdateStatus> future =
        QtConcurrent::run([this]() { return hydra::Updater::NeedsDatabaseUpdate(); });

    QFutureWatcher<hydra::Updater::UpdateStatus>* watcher =
        new QFutureWatcher<hydra::Updater::UpdateStatus>(this);
    connect(watcher, &QFutureWatcher<hydra::Updater::UpdateStatus>::finished, this,
            [this, watcher]() {
                if (watcher->result() == hydra::Updater::UpdateStatus::UpdateAvailable)
                {
                    QMessageBox* msg = new QMessageBox;
                    msg->setWindowTitle("Database update available");
                    msg->setText("There's a new version of the database available.\nWould you like "
                                 "to update in the background?");
                    msg->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msg->setDefaultButton(QMessageBox::Yes);
                    connect(msg, &QMessageBox::finished, this, [this, msg]() {
                        if (msg->clickedButton() == msg->button(QMessageBox::Yes))
                        {
                            statusBar()->showMessage("Updating database...");
                            hydra::Updater::UpdateDatabase(
                                [this]() { statusBar()->showMessage("Database updated"); });
                        }
                    });
                    msg->move(screen()->geometry().center() - frameGeometry().center());
                    msg->show();
                    watcher->deleteLater();
                }
            });
    watcher->setFuture(future);
}

MainWindow::~MainWindow()
{
    ma_device_uninit(&sound_device_);
    stop_emulator();
}

void MainWindow::OpenFile(const std::string& file)
{
    open_file_impl(file);
}

void MainWindow::initialize_audio()
{
    ma_context context;
    ma_context_config context_config = ma_context_config_init();
    context_config.threadPriority = ma_thread_priority_realtime;
    if (ma_context_init(NULL, 0, &context_config, &context) != MA_SUCCESS)
    {
        log_fatal("Failed to initialize audio context");
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.sampleRate = 48000;
    config.periodSizeInFrames = 48000 / 60;
    config.dataCallback = hungry_for_more;
    config.pUserData = this;
    if (ma_device_init(NULL, &config, &sound_device_) != MA_SUCCESS)
    {
        log_fatal("Failed to open audio device");
    }
    ma_device_start(&sound_device_);

    std::string volume_str = Settings::Get("master_volume");
    if (!volume_str.empty())
    {
        int volume = std::stoi(volume_str);
        float volume_f = static_cast<float>(volume) / 100.0f;
        ma_device_set_master_volume(&sound_device_, volume_f);
    }
    else
    {
        ma_device_set_master_volume(&sound_device_, 1.0f);
    }
}

void MainWindow::create_actions()
{
    open_act_ = new QAction(tr("&Open ROM"), this);
    open_act_->setShortcuts(QKeySequence::Open);
    open_act_->setStatusTip(tr("Open a ROM"));
    open_act_->setIcon(QIcon(":/images/open.png"));
    connect(open_act_, &QAction::triggered, this, &MainWindow::open_file);
    settings_act_ = new QAction(tr("&Settings"), this);
    settings_act_->setShortcut(Qt::CTRL | Qt::Key_Comma);
    settings_act_->setStatusTip(tr("Emulator settings"));
    open_settings_file_act_ = new QAction(tr("Open settings file"), this);
    open_settings_file_act_->setStatusTip(tr("Open the settings.json file"));
    connect(open_settings_file_act_, &QAction::triggered, this, []() {
        std::string settings_file = (Settings::GetSavePath() / "settings.json").string();
        QDesktopServices::openUrl(QUrl::fromLocalFile(settings_file.c_str()));
    });
    open_settings_folder_act_ = new QAction(tr("Open settings folder"), this);
    open_settings_folder_act_->setStatusTip(tr("Open the settings folder"));
    connect(open_settings_folder_act_, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(Settings::GetSavePath().string().c_str()));
    });
    connect(settings_act_, &QAction::triggered, this, &MainWindow::open_settings);
    close_act_ = new QAction(tr("&Exit"), this);
    close_act_->setShortcut(QKeySequence::Close);
    close_act_->setStatusTip(tr("Exit hydra"));
    connect(close_act_, &QAction::triggered, this, &MainWindow::close);
    pause_act_ = new QAction(tr("&Pause"), this);
    pause_act_->setShortcut(Qt::CTRL | Qt::Key_P);
    pause_act_->setCheckable(true);
    pause_act_->setStatusTip(tr("Pause emulation"));
    connect(pause_act_, &QAction::triggered, this, &MainWindow::pause_emulator);
    stop_act_ = new QAction(tr("&Stop"), this);
    stop_act_->setShortcut(Qt::CTRL | Qt::Key_Q);
    stop_act_->setStatusTip(tr("Stop emulation immediately"));
    connect(stop_act_, &QAction::triggered, this, &MainWindow::stop_emulator);
    mute_act_ = new QAction(tr("&Mute"), this);
    mute_act_->setShortcut(Qt::CTRL | Qt::Key_M);
    mute_act_->setCheckable(true);
    mute_act_->setStatusTip(tr("Mute audio"));
    connect(mute_act_, &QAction::triggered, this, [this]() {
        if (mute_act_->isChecked())
        {
            ma_device_set_master_volume(&sound_device_, 0.0f);
        }
        else
        {
            std::string volume_str = Settings::Get("master_volume");
            if (!volume_str.empty())
            {
                int volume = std::stoi(volume_str);
                float volume_f = static_cast<float>(volume) / 100.0f;
                ma_device_set_master_volume(&sound_device_, volume_f);
            }
            else
            {
                ma_device_set_master_volume(&sound_device_, 1.0f);
            }
        }
    });
    reset_act_ = new QAction(tr("&Reset"), this);
    reset_act_->setShortcut(Qt::CTRL | Qt::Key_R);
    reset_act_->setStatusTip(tr("Reset emulator"));
    connect(reset_act_, &QAction::triggered, this, &MainWindow::reset_emulator);
    screenshot_act_ = new QAction(tr("S&creenshot"), this);
    screenshot_act_->setShortcut(Qt::Key_F12);
    screenshot_act_->setStatusTip(tr("Take a screenshot (check settings for save path)"));
    connect(screenshot_act_, &QAction::triggered, this, &MainWindow::screenshot);
    about_act_ = new QAction(tr("&About"), this);
    about_act_->setShortcut(QKeySequence::HelpContents);
    about_act_->setStatusTip(tr("Show about dialog"));
    connect(about_act_, &QAction::triggered, this, &MainWindow::open_about);
    shaders_act_ = new QAction(tr("&Shaders"), this);
    shaders_act_->setShortcut(Qt::Key_F11);
    shaders_act_->setStatusTip("Open the shader editor");
    shaders_act_->setIcon(QIcon(":/images/shaders.png"));
    connect(shaders_act_, &QAction::triggered, this, &MainWindow::open_shaders);
    scripts_act_ = new QAction(tr("S&cripts"), this);
    scripts_act_->setShortcut(Qt::Key_F10);
    scripts_act_->setStatusTip("Open the script editor");
    scripts_act_->setIcon(QIcon(":/images/scripts.png"));
    connect(scripts_act_, &QAction::triggered, this, &MainWindow::open_scripts);
#ifndef HYDRA_USE_LUA
    scripts_act_->setEnabled(false);
#endif
    terminal_act_ = new QAction(tr("&Terminal"), this);
    terminal_act_->setShortcut(Qt::Key_F9);
    terminal_act_->setStatusTip("Open the terminal");
    terminal_act_->setIcon(QIcon(":/images/terminal.png"));
    connect(terminal_act_, &QAction::triggered, this, &MainWindow::open_terminal);
    cheats_act_ = new QAction(tr("&Cheats"), this);
    cheats_act_->setShortcut(Qt::Key_F8);
    cheats_act_->setStatusTip("Open the cheats window");
    connect(cheats_act_, &QAction::triggered, this, &MainWindow::toggle_cheats_window);
    recent_act_ = new QAction(tr("&Recent files"), this);
    for (int i = 0; i < 10; i++)
    {
        std::string path = Settings::Get("recent_" + std::to_string(i));
        if (!path.empty())
        {
            recent_files_.push_back(path);
        }
        else
        {
            break;
        }
    }
    if (recent_files_.size() == 0)
    {
        QMenu* empty_menu = new QMenu;
        QAction* empty_act = new QAction(tr("No recent files"));
        empty_act->setEnabled(false);
        empty_menu->addAction(empty_act);
        recent_act_->setMenu(empty_menu);
    }
    else
    {
        update_recent_files();
    }
}

void MainWindow::create_menus()
{
    file_menu_ = menuBar()->addMenu(tr("&File"));
    file_menu_->addAction(open_act_);
    file_menu_->addAction(recent_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(screenshot_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(open_settings_file_act_);
    file_menu_->addAction(open_settings_folder_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(settings_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(close_act_);
    emulation_menu_ = menuBar()->addMenu(tr("&Emulation"));
    emulation_menu_->addAction(pause_act_);
    emulation_menu_->addAction(reset_act_);
    emulation_menu_->addAction(stop_act_);
    emulation_menu_->addSeparator();
    emulation_menu_->addAction(mute_act_);
    tools_menu_ = menuBar()->addMenu(tr("&Tools"));
    tools_menu_->addAction(terminal_act_);
    tools_menu_->addAction(cheats_act_);
    tools_menu_->addAction(scripts_act_);
    tools_menu_->addAction(shaders_act_);
    help_menu_ = menuBar()->addMenu(tr("&Help"));
    help_menu_->addAction(about_act_);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (!emulator_)
        return;
    if (backwards_mappings_.find(event->key()) == backwards_mappings_.end())
        return;
    auto [player, key] = backwards_mappings_[event->key()];
    input_state_[player][(int)key] = 1;
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (!emulator_)
        return;
    if (backwards_mappings_.find(event->key()) == backwards_mappings_.end())
        return;
    auto [player, key] = backwards_mappings_[event->key()];
    input_state_[player][(int)key] = 0;
}

void MainWindow::on_mouse_move(QMouseEvent* event) {}

void MainWindow::on_mouse_click(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        mouse_state_ = (event->pos().x() << 16) | (event->pos().y() & 0xFFFF);
    }
}

void MainWindow::on_mouse_release(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        mouse_state_ = hydra::TOUCH_RELEASED;
    }
}

void MainWindow::open_file()
{
    static QString extensions;
    if (extensions.isEmpty())
    {
        QString indep;
        extensions = "All supported types (";
        for (size_t i = 0; i < Settings::CoreInfo.size(); i++)
        {
            const auto& core_data = Settings::CoreInfo[i];
            indep += core_data.core_name.c_str();
            indep += " (";
            for (size_t j = 0; j < core_data.extensions.size(); j++)
            {
                std::string ext = core_data.extensions[j];
                extensions += "*.";
                extensions += ext.c_str();
                if (j != core_data.extensions.size() - 1)
                    extensions += " ";
                else if (i != Settings::CoreInfo.size() - 1)
                    extensions += " ";
                indep += "*.";
                indep += ext.c_str();
                if (j != core_data.extensions.size() - 1)
                    indep += " ";
            }
            if (i != Settings::CoreInfo.size() - 1)
                indep += ");;";
            else
                indep += ")";
        }
        extensions += ")";
        if (indep != "")
            extensions += ";;" + indep;
    }
    std::string last_path = Settings::Get("last_path");
    QFileDialog dialog(this, tr("Open File"), last_path.c_str(), extensions);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.show();
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }
    std::string path = dialog.selectedFiles().first().toStdString();
    qt_may_throw(std::bind(&MainWindow::open_file_impl, this, path));
}

void MainWindow::open_file_impl(const std::string& path)
{
    std::unique_lock<std::mutex> elock(emulator_mutex_);
    emulator_timer_->stop();
    std::filesystem::path pathfs(path);

    if (!std::filesystem::is_regular_file(pathfs))
    {
        log_warn(fmt::format("Failed to open file: {}", path).c_str());
        return;
    }

    stop_emulator();
    Settings::Set("last_path", pathfs.parent_path().string());
    std::string core_path;
    for (const auto& core : Settings::CoreInfo)
    {
        for (const auto& ext : core.extensions)
        {
            if (pathfs.extension().string().substr(1) == ext)
            {
                core_path = core.path;
                break;
            }
        }
        if (!core_path.empty())
            break;
    }
    if (core_path.empty())
    {
        log_warn(fmt::format("Failed to find core for file: {}", path).c_str());
        return;
    }
    info_.reset();
    for (auto& info : Settings::CoreInfo)
    {
        if (info.path == core_path)
        {
            info_ = std::make_unique<EmulatorInfo>(info);
            break;
        }
    }
    if (!info_)
    {
        log_warn(
            fmt::format("Failed to find core info for core {}... This shouldn't happen?", core_path)
                .c_str());
    }
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        std::ifstream file(path, std::ifstream::binary);
        MD5_CTX md5Context;
        MD5_Init(&md5Context);
        char buf[1024 * 16];
        while (file.good())
        {
            file.read(buf, sizeof(buf));
            MD5_Update(&md5Context, buf, file.gcount());
        }
        MD5_Final(result, &md5Context);
        std::stringstream md5stream;
        md5stream << std::hex << std::setfill('0');
        for (const auto& byte : result)
        {
            md5stream << std::setw(2) << (int)byte;
        }
        game_hash_ = md5stream.str();
    }
    // TODO: such resets don't belong in open_file_impl, but in a separate function
    cheats_window_.reset();
    emulator_ = hydra::EmulatorFactory::Create(core_path);
    if (!emulator_)
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to create emulator");
    init_emulator();
    if (!emulator_->shell->loadFile("rom", path.c_str()))
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to open file");
    enable_emulation_actions(true);
    add_recent(path);

    // Find mappings
    backwards_mappings_.clear();
    input_state_.clear();

    uint32_t max_players = 1;
    if (emulator_->shell->hasInterface(hydra::InterfaceType::IMultiplayer))
    {
        auto multiplayer = emulator_->shell->asIMultiplayer();
        max_players = multiplayer->getMaximumPlayerCount();
    }
    input_state_.resize(max_players);

    for (uint32_t i = 1; i < max_players + 1; i++)
    {
        std::string setting = info_->core_name + "_" + std::to_string(i) + "_mapping";
        std::string mapping = Settings::Get(setting);
        hydra::KeyMappings keys;

        if (mapping == "Default mappings" || mapping.empty())
        {
            QFile file(":/default_mappings.json");
            file.open(QIODevice::ReadOnly);
            keys = hydra::Input::Load(file.readAll().toStdString());
        }
        else
        {
            std::filesystem::path mapping_path =
                Settings::GetSavePath() / "mappings" / (mapping + ".json");
            keys = hydra::Input::Open(mapping_path.string());
        }

        for (int j = 0; j < (int)hydra::ButtonType::InputCount; j++)
        {
            backwards_mappings_[keys[j][0].key()] = {i - 1, (hydra::ButtonType)j};
        }
    }

    paused_ = false;
    emulator_timer_->start();
}

void MainWindow::open_settings()
{
    qt_may_throw([this]() {
        if (!settings_open_)
        {
            using namespace std::placeholders;
            new SettingsWindow(settings_open_, std::bind(&MainWindow::set_volume, this, _1), this);
        }
    });
}

void MainWindow::open_shaders()
{
    qt_may_throw([this]() {
        if (!shaders_open_)
        {
            // using namespace std::placeholders;
            // std::function<void(QString*, QString*)> callback =
            //     std::bind(&ScreenWidget::ResetProgram, screen_, _1, _2);
            // new ShaderEditor(shaders_open_, callback, this);
        }
    });
}

void MainWindow::open_scripts()
{
    qt_may_throw([this]() {
        if (!scripts_open_)
        {
            using namespace std::placeholders;
            new ScriptEditor(scripts_open_, std::bind(&MainWindow::run_script, this, _1, _2), this);
        }
    });
}

void MainWindow::open_terminal()
{
    qt_may_throw([this]() {
        if (!terminal_open_)
        {
            new TerminalWindow(terminal_open_, this);
        }
    });
}

void MainWindow::toggle_cheats_window()
{
    qt_may_throw([this]() {
        if (!cheats_window_)
        {
            cheats_window_.reset(new CheatsWindow(emulator_, cheats_open_, game_hash_, this));
        }
        else
        {
            if (!cheats_open_)
            {
                cheats_window_->Show();
            }
            else
            {
                cheats_window_->Hide();
            }
        }
    });
}

// TODO: compiler option to turn off lua support
void MainWindow::run_script(const std::string& script, bool safe_mode)
{
#ifdef HYDRA_USE_LUA
    static bool initialized = false;
    static sol::state lua;
    static auto environment = sol::environment(lua, sol::create);
    if (!initialized)
    {
        lua.open_libraries();
        const std::vector<std::string> whitelisted = {
            "assert", "error",    "ipairs",   "next", "pairs",  "pcall",  "print",
            "select", "tonumber", "tostring", "type", "unpack", "xpcall",
        };

        for (const auto& name : whitelisted)
        {
            environment[name] = lua[name];
        }

        std::vector<std::string> libraries = {"coroutine", "string", "table", "math"};

        for (const auto& library : libraries)
        {
            sol::table copy(lua, sol::create);
            for (auto [name, func] : lua[library].tbl)
            {
                copy[name] = func;
            }
            environment[library] = copy;
        }

        sol::table os(lua, sol::create);
        os["clock"] = lua["os"]["clock"];
        os["date"] = lua["os"]["date"];
        os["difftime"] = lua["os"]["difftime"];
        os["time"] = lua["os"]["time"];
        environment["os"] = os;
        initialized = true;
    }

    qt_may_throw([this, &script, &safe_mode]() {
        if (safe_mode)
        {
            lua.script(script, environment);
        }
        else
        {
            lua.script(script);
        }
    });
#endif
}

void MainWindow::screenshot()
{
    std::unique_lock<std::mutex> elock(emulator_mutex_);
    std::filesystem::path screenshot_path = Settings::Get("screenshot_path");
    if (screenshot_path.empty())
    {
        screenshot_path = std::filesystem::current_path();
    }

    int screenshot_index = 0;
    std::string screenshot_name = "hydra_screenshot";
    std::string screenshot_extension = ".png";
    while (std::filesystem::exists(screenshot_path / (screenshot_name + screenshot_extension)))
    {
        screenshot_index++;
        screenshot_name = "hydra_screenshot_" + std::to_string(screenshot_index);
    }

    std::filesystem::path screenshot_full_path =
        screenshot_path / (screenshot_name + screenshot_extension);
    QImage image = screen_->grabFramebuffer();
    image.save(screenshot_full_path.string().c_str());
}

void MainWindow::set_volume(int volume)
{
    ma_device_set_master_volume(&sound_device_, volume / 100.0f);
}

void MainWindow::open_about()
{
    qt_may_throw([this]() {
        if (!about_open_)
        {
            new AboutWindow(about_open_, this);
        }
    });
}

void MainWindow::enable_emulation_actions(bool should)
{
    stop_act_->setEnabled(should);
    reset_act_->setEnabled(should);
    scripts_act_->setEnabled(should);
    terminal_act_->setEnabled(should);
    cheats_act_->setEnabled(should);
    shaders_act_->setEnabled(should);
    if (should)
        screen_->show();
    else
        screen_->hide();
}

void MainWindow::pause_emulator()
{
    paused_ = !paused_;
    if (paused_)
    {
        emulator_timer_->stop();
    }
    else
    {
        emulator_timer_->start();
    }
}

void MainWindow::reset_emulator()
{
    if (emulator_)
    {
        std::unique_lock<std::mutex> alock(audio_mutex_);
        queued_audio_.clear();
        emulator_->shell->asIBase()->reset();
    }
}

using fptr = void (*)();

fptr get_proc_address(const char* name)
{
    return QOpenGLContext::currentContext()->getProcAddress(name);
}

void poll_input_callback() {}

void MainWindow::init_emulator()
{
    if (!emulator_->shell)
    {
        log_fatal("Emulator not loaded correctly?");
    }

    if (!emulator_->shell->hasInterface(hydra::InterfaceType::IBase))
    {
        log_fatal("Emulator does not have base interface?");
    }

    // Initialize gl
    if (emulator_->shell->hasInterface(hydra::InterfaceType::IOpenGlRendered))
    {
        hydra::IOpenGlRendered* shell_gl = emulator_->shell->asIOpenGlRendered();
        shell_gl->setGetProcAddress((void*)get_proc_address);
        shell_gl->resetContext();
        // TODO: tf so ugly
        auto size = emulator_->shell->getNativeSize();
        screen_->Resize(size.width, size.height);
        // TODO: better resizing!!
        resize(size.width, size.height);
        if (screen_->GetFbo() == 0)
        {
            log_fatal("FBO not initialized correctly?");
        }
        shell_gl->setFbo(screen_->GetFbo());
        emulator_->shell->setOutputSize(size);
    }

    // Initialize sw
    if (emulator_->shell->hasInterface(hydra::InterfaceType::ISoftwareRendered))
    {
        hydra::ISoftwareRendered* shell_sw = emulator_->shell->asISoftwareRendered();
        shell_sw->setVideoCallback(video_callback);
    }

    // Initialize audio
    if (emulator_->shell->hasInterface(hydra::InterfaceType::IAudio))
    {
        hydra::IAudio* shell_audio = emulator_->shell->asIAudio();
        shell_audio->setSampleRate(48000);
        shell_audio->setAudioCallback(audio_callback);
    }

    // Initialize input
    if (emulator_->shell->hasInterface(hydra::InterfaceType::IInput))
    {
        hydra::IInput* shell_input = emulator_->shell->asIInput();
        shell_input->setPollInputCallback(poll_input_callback);
        shell_input->setCheckButtonCallback(read_input_callback);
    }

    // Initialize logging
    if (emulator_->shell->hasInterface(hydra::InterfaceType::ILog))
    {
        hydra::ILog* shell_log = emulator_->shell->asILog();
        shell_log->setLogCallback(hydra::LogTarget::Warning, TerminalWindow::log_warn);
        shell_log->setLogCallback(hydra::LogTarget::Info, TerminalWindow::log_info);
        shell_log->setLogCallback(hydra::LogTarget::Debug, TerminalWindow::log_debug);
        shell_log->setLogCallback(hydra::LogTarget::Error, log_fatal);
        terminal_act_->setEnabled(true);
    }
    else
    {
        terminal_act_->setEnabled(false);
    }

    // Initialize cheats
    if (emulator_->shell->hasInterface(hydra::InterfaceType::ICheat))
    {
        cheats_act_->setEnabled(true);
    }
    else
    {
        cheats_act_->setEnabled(false);
    }

    if (emulator_->shell->hasInterface(hydra::InterfaceType::ISelfDriven))
    {
        printf("Warning: self driven cores are not supported currently and the API for them is "
               "bound to change");
    }

    // Initialize firmware
    std::vector<std::string> firmware_files = info_->firmware_files;
    for (const auto& file : firmware_files)
    {
        std::string core_name = info_->core_name;
        std::string path = Settings::Get(core_name + "_" + file);
        if (path.empty())
        {
            log_fatal(fmt::format("Firmware file {} not set in settings", file).c_str());
        }
        emulator_->shell->loadFile(file.c_str(), path.c_str());
    }
}

void MainWindow::stop_emulator()
{
    if (emulator_)
    {
        std::unique_lock<std::mutex> alock(audio_mutex_);
        emulator_.reset();
        std::fill(video_buffer_.begin(), video_buffer_.end(), 0);
        enable_emulation_actions(false);
    }
}

// TODO: check if frontend driven core
void MainWindow::emulator_frame()
{
    std::unique_lock<std::mutex> elock(emulator_mutex_);
    frame_count_++;
    if (emulator_->shell->hasInterface(hydra::InterfaceType::IOpenGlRendered))
    {
        hydra::IOpenGlRendered* shell_gl = emulator_->shell->asIOpenGlRendered();
        shell_gl->setFbo(screen_->GetFbo());
        auto size = emulator_->shell->getNativeSize();
        screen_->Resize(size.width, size.height);
    }
    else if (emulator_->shell->hasInterface(hydra::InterfaceType::ISoftwareRendered))
    {
        // TODO: rename variables to something more meaningful
        screen_->Resize(video_width_, video_height_);
        screen_->Redraw(video_buffer_.data());
    }
    emulator_->shell->asIFrontendDriven()->runFrame();
    screen_->update();
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_));
    if (frame_count_ >= emulator_->shell->asIFrontendDriven()->getFps())
    {
        frame_count_ = 0;
        // Check how many ms elapsed
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - last_emulation_second_time_)
                .count();
        if (elapsed > 1100)
        {
            sleep_time_ -= 1;
        }
        else if (elapsed < 900)
        {
            sleep_time_ += 1;
        }
        last_emulation_second_time_ = std::chrono::high_resolution_clock::now();
    }
}

void MainWindow::video_callback(void* data, hydra::Size size)
{
    main_window->video_width_ = size.width;
    main_window->video_height_ = size.height;
    if (data)
    {
        main_window->video_buffer_.resize(size.width * size.height * 4);
        std::memcpy(main_window->video_buffer_.data(), data, main_window->video_buffer_.size());
    }
}

void MainWindow::audio_callback(void* data, size_t frames)
{
    std::unique_lock<std::mutex> lock(main_window->audio_mutex_);
    main_window->queued_audio_.reserve(main_window->queued_audio_.size() + frames * 2);
    // TODO: fix for float samples
    main_window->queued_audio_.insert(main_window->queued_audio_.end(), (int16_t*)data,
                                      (int16_t*)data + frames * 2);
}

int32_t MainWindow::read_input_callback(uint32_t player, hydra::ButtonType button)
{
    // TODO: is there such a thing as multiplayer touch?
    if (button == hydra::ButtonType::Touch)
        return main_window->mouse_state_;

    return main_window->input_state_[player][(int)button];
}

void MainWindow::add_recent(const std::string& path)
{
    auto it = std::find(recent_files_.begin(), recent_files_.end(), path);
    if (it != recent_files_.end())
    {
        recent_files_.erase(it);
    }
    recent_files_.push_front(path);
    if (recent_files_.size() > 10)
    {
        recent_files_.pop_back();
    }

    update_recent_files();
}

void MainWindow::update_recent_files()
{
    QMenu* menu = new QMenu;
    for (size_t i = 0; i < recent_files_.size(); i++)
    {
        Settings::Set("recent_" + std::to_string(i), recent_files_[i]);
        std::string path = recent_files_[i];
        menu->addAction(path.c_str(), [this, path]() { open_file_impl(path); });
    }
    if (recent_act_->menu())
        recent_act_->menu()->deleteLater();
    recent_act_->setMenu(menu);
}
