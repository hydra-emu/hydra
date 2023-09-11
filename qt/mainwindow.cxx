#include "mainwindow.hxx"
#include "aboutwindow.hxx"
#include "emulator_tool_factory.hxx"
#include "qthelper.hxx"
#include "settingswindow.hxx"
#include "shadereditor.hxx"
#include <emulator_settings.hxx>
#include <error_factory.hxx>
#include <fstream>
#include <iostream>
#include <QApplication>
#include <QClipboard>
#include <QGridLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QSurfaceFormat>
#include <QTimer>

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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), tools_{}, tools_open_{}
{
    setup_emulator_specific();
    QWidget* widget = new QWidget;
    setCentralWidget(widget);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->setContentsMargins(5, 5, 5, 5);
    screen_ = new ScreenWidget(this);
    screen_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    screen_->setVisible(true);
    screen_->setMinimumSize(1, 1);
    layout->addWidget(screen_);
    widget->setLayout(layout);
    create_actions();
    create_menus();
    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);
    setMinimumSize(160, 160);
    resize(640, 480);
    setWindowTitle("hydra");
    setWindowIcon(QIcon(":/images/hydra.png"));

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(emulator_frame()));
    timer->start();
    enable_emulation_actions(false);
    screen_->SetMouseMoveCallback([this](QMouseEvent* event) { on_mouse_move(event); });
    screen_->setMouseTracking(true);

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

MainWindow::~MainWindow()
{
    ma_device_uninit(&sound_device_);
    stop_emulator();
}

void MainWindow::OpenFile(const std::string& file)
{
    open_file_impl(file);
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
    reset_act_->setStatusTip(tr("Soft reset emulator"));
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
    tools_actions_[ET_Debugger] = new QAction(tr("&Debugger"), this);
    tools_actions_[ET_Debugger]->setShortcut(Qt::Key_F2);
    tools_actions_[ET_Debugger]->setStatusTip("Open the debugger");
    tools_actions_[ET_Debugger]->setIcon(QIcon(":/images/debugger.png"));
    connect(tools_actions_[ET_Debugger], &QAction::triggered, this,
            std::bind(&MainWindow::open_tool, this, ET_Debugger));
    tools_actions_[ET_Tracelogger] = new QAction(tr("&Tracelogger"), this);
    tools_actions_[ET_Tracelogger]->setShortcut(Qt::Key_F3);
    tools_actions_[ET_Tracelogger]->setStatusTip("Open the tracelogger");
    tools_actions_[ET_Tracelogger]->setIcon(QIcon(":/images/tracelogger.png"));
    connect(tools_actions_[ET_Tracelogger], &QAction::triggered, this,
            std::bind(&MainWindow::open_tool, this, ET_Tracelogger));
    tools_actions_[ET_MmioViewer] = new QAction(tr("&Mmio Viewer"), this);
    tools_actions_[ET_MmioViewer]->setShortcut(Qt::Key_F4);
    tools_actions_[ET_MmioViewer]->setStatusTip("Open the MMIO viewer");
    tools_actions_[ET_MmioViewer]->setIcon(QIcon(":/images/mmioviewer.png"));
    connect(tools_actions_[ET_MmioViewer], &QAction::triggered, this,
            std::bind(&MainWindow::open_tool, this, ET_MmioViewer));
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
    tools_menu_->addSeparator();
    for (size_t i = 0; i < EmulatorToolsSize; i++)
    {
        tools_menu_->addAction(tools_actions_[i]);
    }
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
            const auto& emulator_data =
                EmulatorSettings::GetEmulatorData(static_cast<hydra::EmuType>(i));
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
    std::string lastpath = "";
    if (EmulatorSettings::GetGeneralSettings().Has("last_path"))
    {
        lastpath = EmulatorSettings::GetGeneralSettings().Get("last_path");
    }
    std::string path = QFileDialog::getOpenFileName(this, tr("Open ROM"),
                                                    QString::fromStdString(lastpath), extensions)
                           .toStdString();
    if (path.empty())
    {
        return;
    }
    open_file_impl(path);
}

void MainWindow::open_file_impl(const std::string& path)
{
    std::filesystem::path pathfs(path);
    if (!std::filesystem::is_regular_file(pathfs))
    {
        return;
    }
    std::string dirpath = pathfs.parent_path().string();
    EmulatorSettings::GetGeneralSettings().Set("last_path", dirpath);
    close_tools();
    Logger::ClearWarnings();
    auto type = hydra::EmulatorFactory::GetEmulatorType(path);
    {
        stop_emulator();
        auto emulator = hydra::EmulatorFactory::Create(type);
        std::swap(emulator, emulator_);
        // Old emulator is destroyed here
    }
    if (!emulator_->LoadFile("rom", path))
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to open ROM");
    screen_->InitializeTexture(640, 320);
    screen_->show();
    emulator_type_ = type;
    enable_emulation_actions(true);
    for (size_t i = 0; i < tools_.size(); i++)
    {
        if (tools_[i])
        {
            delete tools_[i];
        }
        tools_[i] = nullptr;
    }

    std::fill(std::begin(tools_open_), std::end(tools_open_), false);
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

void MainWindow::open_tool(EmulatorTool tool)
{
    qt_may_throw([this, tool]() {
        if (!tools_open_[tool])
        {
            if (tools_[tool])
            {
                if (tools_[tool]->isHidden())
                {
                    tools_[tool]->show();
                }
                else
                {
                    tools_[tool]->hide();
                }
            }
            else
            {
                // auto qw = EmulatorToolFactory::CreateTool(tool, tools_open_[tool],
                // emulator_type_,
                //                                           emulator_.get());
                // tools_[tool] = qw;
            }
        }
    });
}

void MainWindow::screenshot()
{
    // QApplication::clipboard()->setImage(texture_.toImage());
}

void MainWindow::close_tools() {}

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
    std::fill(std::begin(tools_open_), std::end(tools_open_), false);
    if (should)
    {
        const auto& emulator_data = EmulatorSettings::GetEmulatorData(emulator_type_);
        tools_actions_[ET_Debugger]->setEnabled(emulator_data.HasDebugger);
        tools_actions_[ET_Tracelogger]->setEnabled(emulator_data.HasTracelogger);
        tools_actions_[ET_MmioViewer]->setEnabled(true);
    }
}

void MainWindow::setup_emulator_specific()
{
    QFile f(":/emulators.json");
    if (!f.open(QIODevice::ReadOnly))
    {
        throw ErrorFactory::generate_exception(__func__, __LINE__,
                                               "Could not open default emulators.json");
    }
    auto mappings_path = hydra::EmulatorFactory::GetSavePath() + "mappings.json";
    QString data_mappings;
    if (std::filesystem::exists(mappings_path))
    {
        QFile f2(mappings_path.c_str());
        if (!f2.open(QIODevice::ReadOnly))
        {
            throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                   "Could not open mappings.json");
        }
        data_mappings = f2.readAll();
    }
    else
    {
        QFile f2(":/mappings.json");
        if (!f2.open(QIODevice::ReadOnly))
        {
            throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                   "Could not open default mappings.json");
        }
        data_mappings = f2.readAll();
        std::ofstream ofs(mappings_path);
        if (ofs.is_open())
        {
            ofs << data_mappings.toStdString();
            ofs.close();
        }
        else
        {
            throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                   "Could not open mappings.json");
        }
    }
    QString options_str = f.readAll();
    json json_options = json::parse(options_str.toStdString());
    json json_mappings = json::parse(data_mappings.toStdString());
    for (auto it = json_options.begin(); it != json_options.end(); ++it)
    {
        EmulatorData d;
        json& o = it.value();
        o.at("Name").get_to(d.Name);
        o.at("SettingsFile").get_to(d.SettingsFile);
        o.at("Extensions").get_to(d.Extensions);
        o.at("HasDebugger").get_to(d.HasDebugger);
        o.at("HasTracelogger").get_to(d.HasTracelogger);
        o.at("LoggingOptions").get_to(d.LoggingOptions);
        d.UserData = EmulatorUserData(hydra::EmulatorFactory::GetSavePath() + d.SettingsFile);
        EmulatorData& constant_settings =
            EmulatorSettings::GetEmulatorData(static_cast<hydra::EmuType>(std::stoi(it.key())));
        constant_settings = d;
    }
    for (auto it = json_mappings.begin(); it != json_mappings.end(); ++it)
    {
        auto& d =
            EmulatorSettings::GetEmulatorData(static_cast<hydra::EmuType>(std::stoi(it.key())));
        json& o = it.value();
        o.get_to(d.Mappings);
    }
    for (int i = 0; i < static_cast<int>(hydra::EmuType::EmuTypeSize); i++)
    {
        auto& d = EmulatorSettings::GetEmulatorData(static_cast<hydra::EmuType>(i));
        for (auto& mapping : d.Mappings)
        {
            mapping.second =
                std::to_string((QKeySequence::fromString(mapping.second.c_str()))[0].key());
        }
    }
    // Write default emulator options if they dont exist
    for (int i = 0; i < EmuTypeSize; i++)
    {
        const auto& e = EmulatorSettings::GetEmulatorData(static_cast<hydra::EmuType>(i));
        if (!std::filesystem::exists(hydra::EmulatorFactory::GetSavePath() + e.SettingsFile))
        {
            std::ofstream ofs(hydra::EmulatorFactory::GetSavePath() + e.SettingsFile);
            if (ofs.is_open())
            {
                QFile resource(std::string(":/" + e.SettingsFile).c_str());
                if (!resource.open(QIODeviceBase::ReadOnly))
                {
                    throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                           "Could not open resource file");
                }
                ofs << resource.readAll().toStdString();
                ofs.close();
            }
            else
            {
                throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                       "Could not create default options file");
            }
        }
    }
    // Read emulator options
    for (int i = 0; i < EmuTypeSize; i++)
    {
        std::map<std::string, std::string> temp;
        EmulatorUserData& user_data =
            EmulatorSettings::GetEmulatorData(static_cast<hydra::EmuType>(i)).UserData;
        auto path = hydra::EmulatorFactory::GetSavePath() +
                    EmulatorSettings::GetEmulatorData(static_cast<hydra::EmuType>(i)).SettingsFile;
        std::ifstream ifs(path);
        if (ifs.is_open())
        {
            std::stringstream buf;
            buf << ifs.rdbuf();
            json j = json::parse(buf.str());
            for (auto it = j.begin(); it != j.end(); ++it)
            {
                user_data.Set(it.key(), it.value());
            }
        }
        else
        {
            throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                   "Could not open emulator options file");
        }
    }
    // Read general options
    {
        auto path = hydra::EmulatorFactory::GetSavePath() + "settings.json";
        std::map<std::string, std::string> temp;
        if (std::filesystem::exists(path))
        {
            std::ifstream ifs(path);
            if (ifs.is_open())
            {
                std::stringstream buf;
                buf << ifs.rdbuf();
                json json_general = json::parse(buf.str());
                for (auto it = json_general.begin(); it != json_general.end(); ++it)
                {
                    temp[it.key()] = it.value();
                }
            }
            else
            {
                throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                       "Could not open options file");
            }
        }
    }
}

void MainWindow::pause_emulator()
{
    paused_ = !paused_;
}

void MainWindow::reset_emulator()
{
    if (emulator_)
        emulator_->Reset();
}

void MainWindow::stop_emulator()
{
    if (emulator_)
    {
        emulator_.reset();
        enable_emulation_actions(false);
    }
    screen_->hide();
}

void MainWindow::emulator_frame()
{
    if (!emulator_ || paused_)
    {
        return;
    }

    std::future<void> frame = emulator_->RunFrameAsync();
    frame.wait();

    std::future<hydra::VideoInfo> video_info = emulator_->RenderFrameAsync();
    std::future<hydra::AudioInfo> audio_info = emulator_->RenderAudioAsync();
    video_info.wait();
    audio_info.wait();

    hydra::VideoInfo vi = video_info.get();
    hydra::AudioInfo ai = audio_info.get();

    screen_->Redraw(vi.width, vi.height, vi.data.data());
    screen_->update();

    std::unique_lock<std::mutex> lock(audio_mutex_);
    queued_audio_.reserve(queued_audio_.size() + ai.data.size());
    queued_audio_.insert(queued_audio_.end(), ai.data.begin(), ai.data.end());
}
