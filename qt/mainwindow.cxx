#include "mainwindow.hxx"
#include "aboutwindow.hxx"
#include "qthelper.hxx"
#include "settingswindow.hxx"
#include "shadereditor.hxx"
#include <error_factory.hxx>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <log.hxx>
#include <QApplication>
#include <QClipboard>
#include <QGridLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QSurfaceFormat>
#include <QTimer>
#include <settings.hxx>

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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    QWidget* widget = new QWidget;
    setCentralWidget(widget);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    screen_ = new ScreenWidget(this);
    screen_->SetMouseMoveCallback([this](QMouseEvent* event) { on_mouse_move(event); });
    screen_->setMouseTracking(true);
    layout->addWidget(screen_);
    widget->setLayout(layout);
    create_actions();
    create_menus();

    QString message = tr("Welcome to hydra!");
    statusBar()->showMessage(message);
    setMinimumSize(160, 160);
    resize(640, 480);
    setWindowTitle("hydra");
    setWindowIcon(QIcon(":/images/hydra.png"));

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(emulator_frame()));
    timer->start();

    initialize_emulator_data();
    initialize_audio();
    enable_emulation_actions(false);
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
        Logger::Fatal("Failed to initialize audio context");
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
        Logger::Fatal("Failed to open audio device");
    }

    ma_device_start(&sound_device_);
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
    connect(settings_act_, &QAction::triggered, this, &MainWindow::open_settings);
    pause_act_ = new QAction(tr("&Pause"), this);
    pause_act_->setShortcut(Qt::CTRL | Qt::Key_P);
    pause_act_->setCheckable(true);
    pause_act_->setStatusTip(tr("Pause emulation"));
    connect(pause_act_, &QAction::triggered, this, &MainWindow::pause_emulator);
    stop_act_ = new QAction(tr("&Stop"), this);
    stop_act_->setShortcut(Qt::CTRL | Qt::Key_Q);
    stop_act_->setStatusTip(tr("Stop emulation immediately"));
    connect(stop_act_, &QAction::triggered, this, &MainWindow::stop_emulator);
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
}

void MainWindow::create_menus()
{
    file_menu_ = menuBar()->addMenu(tr("&File"));
    file_menu_->addAction(open_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(screenshot_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(settings_act_);
    emulation_menu_ = menuBar()->addMenu(tr("&Emulation"));
    emulation_menu_->addAction(pause_act_);
    emulation_menu_->addAction(reset_act_);
    emulation_menu_->addAction(stop_act_);
    tools_menu_ = menuBar()->addMenu(tr("&Tools"));
    tools_menu_->addAction(shaders_act_);
    help_menu_ = menuBar()->addMenu(tr("&Help"));
    help_menu_->addAction(about_act_);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // if (emulator_)
    // {
    //     emulator_->HandleKeyDown(event->key());
    // }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    // if (emulator_)
    // {
    //     emulator_->HandleKeyUp(event->key());
    // }
}

void MainWindow::on_mouse_move(QMouseEvent* event)
{
    // if (emulator_)
    // {
    //     emulator_->HandleMouseMove(event->position().x(), event->position().y());
    // }
}

void MainWindow::open_file()
{
    static QString extensions;
    if (extensions.isEmpty())
    {
        QString indep;
        extensions = "All supported types (";
        for (int i = 0; i < EmuTypeSize; i++)
        {
            const auto& emulator_data = emulator_data_[i];
            indep += emulator_data.Name.c_str();
            indep += " (";
            for (const auto& str : emulator_data.Extensions)
            {
                extensions += "*";
                extensions += str.c_str();
                extensions += " ";
                indep += "*";
                indep += str.c_str();
                indep += " ";
            }
            indep += ");;";
        }
        extensions += ");;";
        extensions += indep;
    }
    std::string last_path = "";
    if (Settings::Has("last_path"))
    {
        last_path = Settings::Get("last_path");
    }
    std::string path =
        QFileDialog::getOpenFileName(this, tr("Open ROM"), QString::fromStdString(last_path),
                                     extensions, nullptr, QFileDialog::ReadOnly)
            .toStdString();
    if (path.empty())
    {
        return;
    }
    qt_may_throw(std::bind(&MainWindow::open_file_impl, this, path));
}

hydra::EmuType MainWindow::get_emulator_type(const std::filesystem::path& path)
{
    for (int i = 0; i < EmuTypeSize; i++)
    {
        const auto& emulator_data = emulator_data_[i];
        for (const auto& str : emulator_data.Extensions)
        {
            if (path.extension() == str)
            {
                return static_cast<hydra::EmuType>(i);
            }
        }
    }

    return hydra::EmuType::EmuTypeSize;
}

void MainWindow::open_file_impl(const std::string& path)
{
    std::unique_lock<std::mutex> elock(emulator_mutex_);
    std::filesystem::path pathfs(path);

    if (!std::filesystem::is_regular_file(pathfs))
    {
        Logger::Warn("Failed to open file: {}", path);
        return;
    }

    Settings::Set("last_path", pathfs.parent_path().string());
    Logger::ClearWarnings();
    auto type = get_emulator_type(path);
    stop_emulator();
    auto emulator = hydra::EmulatorFactory::Create(type);
    std::swap(emulator, emulator_);
    if (!emulator_)
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to create emulator");
    emulator_->SetVideoCallback(
        std::bind(&MainWindow::video_callback, this, std::placeholders::_1));
    emulator_->SetAudioCallback(
        std::bind(&MainWindow::audio_callback, this, std::placeholders::_1));
    emulator_->SetPollInputCallback(std::bind(&MainWindow::poll_input_callback, this));
    emulator_->SetReadInputCallback(
        std::bind(&MainWindow::read_input_callback, this, std::placeholders::_1));
    if (!emulator_->LoadFile("rom", path))
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to open ROM");
    emulator_type_ = type;
    enable_emulation_actions(true);
    paused_ = false;
}

void MainWindow::open_settings()
{
    qt_may_throw([this]() {
        if (!settings_open_)
        {
            new SettingsWindow(settings_open_, this);
        }
    });
}

void MainWindow::open_shaders()
{
    qt_may_throw([this]() {
        if (!shaders_open_)
        {
            using namespace std::placeholders;
            std::function<void(QString*, QString*)> callback =
                std::bind(&ScreenWidget::ResetProgram, screen_, _1, _2);
            new ShaderEditor(shaders_open_, callback, this);
        }
    });
}

void MainWindow::screenshot()
{
    // QApplication::clipboard()->setImage(texture_.toImage());
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
    pause_act_->setEnabled(should);
    stop_act_->setEnabled(should);
    reset_act_->setEnabled(should);
    screen_->setVisible(should);
}

void MainWindow::initialize_emulator_data()
{
    auto settings_path = hydra::EmulatorFactory::GetSavePath() + "settings.json";
    Settings::Open(settings_path);

    for (int i = 0; i < EmuTypeSize; i++)
    {
        std::string file_name = hydra::serialize_emu_type(static_cast<hydra::EmuType>(i));
        std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
        auto path = ":/emulators/" + file_name + ".json";

        QFile file(QString::fromStdString(path));
        if (!file.open(QIODevice::ReadOnly))
        {
            Logger::Fatal("Failed to open emulator data file: {}", path);
        }
        std::string data = file.readAll().toStdString();

        using json = nlohmann::json;
        json j = json::parse(data);
        emulator_data_[i].Name = j["Name"].get<std::string>();
        emulator_data_[i].Extensions = j["Extensions"].get<std::vector<std::string>>();
    }
}

void MainWindow::pause_emulator()
{
    paused_ = !paused_;
}

void MainWindow::reset_emulator()
{
    if (emulator_)
    {
        std::unique_lock<std::mutex> alock(audio_mutex_);
        queued_audio_.clear();
        emulator_->Reset();
    }
}

void MainWindow::stop_emulator()
{
    if (emulator_)
    {
        std::unique_lock<std::mutex> alock(audio_mutex_);
        queued_audio_.clear();
        emulator_.reset();
        enable_emulation_actions(false);
        video_info_ = {};
    }
}

void MainWindow::emulator_frame()
{
    std::unique_lock<std::mutex> elock(emulator_mutex_);
    if (!emulator_ || paused_)
    {
        return;
    }

    std::future<void> frame = emulator_->RunFrameAsync();
    frame.wait();

    screen_->Redraw(video_info_.width, video_info_.height, video_info_.data.data());
}

void MainWindow::video_callback(const hydra::VideoInfo& vi)
{
    video_info_ = vi;
}

void MainWindow::audio_callback(const hydra::AudioInfo& ai)
{
    std::unique_lock<std::mutex> lock(audio_mutex_);
    queued_audio_.reserve(queued_audio_.size() + ai.data.size());
    queued_audio_.insert(queued_audio_.end(), ai.data.begin(), ai.data.end());
}

void MainWindow::poll_input_callback() {}

int8_t MainWindow::read_input_callback(const hydra::InputInfo& ii)
{
    return 0;
}