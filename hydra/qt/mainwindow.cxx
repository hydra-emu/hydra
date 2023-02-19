#include "mainwindow.hxx"
#include "settingswindow.hxx"
#include "shadereditor.hxx"
#include "traceloggerwindow.hxx"
#include "aboutwindow.hxx"
#include <include/error_factory.hxx>
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QGridLayout>
#include <QGroupBox>
#include <QSurfaceFormat>
#include <iostream>

#define QT_MAY_THROW(func) try {\
    func \
} catch (std::exception& ex) { \
    QMessageBox messageBox; \
    messageBox.critical(0,"Error", ex.what()); \
    messageBox.setFixedSize(500,200); \
    return; \
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setup_emulator_specific();
    QWidget *widget = new QWidget;
    setCentralWidget(widget);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->setContentsMargins(5, 5, 5, 5);
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);
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

    QTimer *timer = new QTimer;
    timer->start(16);
    connect(timer, SIGNAL(timeout()), this, SLOT(redraw_screen()));

    enable_emulation_actions(false);
}

MainWindow::~MainWindow() {
    stop_emulator();
}

void MainWindow::create_actions() {
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
    debugger_act_ = new QAction(tr("&Debugger"), this);
    debugger_act_->setShortcut(Qt::Key_F2);
    debugger_act_->setStatusTip("Open the debugger");
    debugger_act_->setIcon(QIcon(":/images/debugger.png"));
    connect(debugger_act_, &QAction::triggered, this, &MainWindow::open_debugger);
    tracelogger_act_ = new QAction(tr("&Tracelogger"), this);
    tracelogger_act_->setShortcut(Qt::Key_F3);
    tracelogger_act_->setStatusTip("Open the tracelogger");
    tracelogger_act_->setIcon(QIcon(":/images/tracelogger.png"));
    connect(tracelogger_act_, &QAction::triggered, this, &MainWindow::open_tracelogger);
}

void MainWindow::create_menus() {
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
    tools_menu_->addAction(debugger_act_);
    tools_menu_->addAction(tracelogger_act_);
    help_menu_ = menuBar()->addMenu(tr("&Help"));
    help_menu_->addAction(about_act_);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (emulator_)
        emulator_->HandleKeyDown(event->key());
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (emulator_)
        emulator_->HandleKeyUp(event->key());
}

void MainWindow::open_file() {
    static QString extensions;
    if (extensions.isEmpty()) {
        const auto& data = TKPEmu::EmulatorFactory::GetEmulatorData();
        QString indep;
        extensions = "All supported types (";
        for (int i = 0; i < static_cast<int>(TKPEmu::EmuType::EmuTypeSize); i++) {
            indep += data[i].Name.c_str();
            indep += " (";
            for (const auto& str : data[i].Extensions) {
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
    if (TKPEmu::EmulatorFactory::GetGeneralSettings().Has("last_path"))
    lastpath = TKPEmu::EmulatorFactory::GetGeneralSettings().Get("last_path");
    std::string path = QFileDialog::getOpenFileName(this, tr("Open ROM"),  QString::fromStdString(lastpath), extensions).toStdString();
    if (path.empty())
        return;
    std::filesystem::path pathfs(path);
    if (!std::filesystem::is_regular_file(pathfs))
        return;
    auto dirpath = pathfs.parent_path();
    TKPEmu::EmulatorFactory::GetGeneralSettings().Set("last_path", dirpath);
    QT_MAY_THROW(
        close_tools();
        auto type = TKPEmu::EmulatorFactory::GetEmulatorType(path);
        {
            stop_emulator();
            auto emulator = TKPEmu::EmulatorFactory::Create(type);
            std::swap(emulator, emulator_);
            // Old emulator is destroyed here
        }
        if (!emulator_->LoadFromFile(path))
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to open ROM");
        message_queue_ = emulator_->MessageQueue;
        const auto& data = TKPEmu::EmulatorFactory::GetEmulatorData();
        emulator_->SetWidth(data[static_cast<int>(type)].DefaultWidth);
        emulator_->SetHeight(data[static_cast<int>(type)].DefaultHeight);
        screen_->setMinimumSize(emulator_->GetWidth(), emulator_->GetHeight());
        screen_->InitializeTexture(emulator_->GetWidth(), emulator_->GetHeight(), emulator_->GetBitdepth(), emulator_->GetScreenData());
        emulator_->Paused = pause_act_->isChecked();
        auto func = [&]() {
            emulator_->Start();
        };
        emulator_thread_ = std::thread(func);
        emulator_thread_.detach();
        emulator_type_ = type;
        enable_emulation_actions(true);
        for (int i = 0; i < emulator_tools_.size(); i++) {
            if (emulator_tools_[i])
                delete emulator_tools_[i];
            emulator_tools_[i] = nullptr;
        }
        debugger_open_ = false;
        tracelogger_open_ = false;
    );
}

void MainWindow::open_settings() {
    if (!settings_open_) {
        QT_MAY_THROW(
            auto* qw = new SettingsWindow(settings_open_, this);
        );
    }
}

void MainWindow::open_shaders() {
    if (!shaders_open_) {
        using namespace std::placeholders;
        QT_MAY_THROW(
            std::function<void(QString*, QString*)> callback = std::bind(&ScreenWidget::ResetProgram, screen_, _1, _2);
            auto* qw = new ShaderEditor(shaders_open_, callback, this);
        );
    }
}

void MainWindow::open_debugger() {
    if (!debugger_open_) {
        QT_MAY_THROW(
        );
    }
}

void MainWindow::open_tracelogger() {
    if (!tracelogger_open_) {
        QT_MAY_THROW(
            auto* qw = new TraceloggerWindow(tracelogger_open_, emulator_->MessageQueue, emulator_type_, this);
            emulator_tools_[TraceloggerWindow::GetToolIndex()] = qw;
        );
    }
}

void MainWindow::screenshot() {
    // QApplication::clipboard()->setImage(texture_.toImage());
}

void MainWindow::close_tools() {
    
}

void MainWindow::open_about() {
    if (!about_open_) {
        QT_MAY_THROW(
            auto* qw = new AboutWindow(about_open_, this);
        );
    }
}

void MainWindow::enable_emulation_actions(bool should) {
    pause_act_->setEnabled(should);
    stop_act_->setEnabled(should);
    reset_act_->setEnabled(should);
    if (ScreenWidget::GLInitialized)
        screen_->setVisible(should);
    debugger_act_->setEnabled(false);
    tracelogger_act_->setEnabled(false);
    if (should) {
        const auto& data = TKPEmu::EmulatorFactory::GetEmulatorData().at(static_cast<int>(emulator_type_));
        debugger_act_->setEnabled(data.HasDebugger);
        tracelogger_act_->setEnabled(data.HasTracelogger);
    }
}

void MainWindow::setup_emulator_specific() {
    EmulatorDataMap constant_map;
    EmulatorUserDataMap user_map;
    QFile f(":/data/emulators.json");
    if (!f.open(QIODevice::ReadOnly)) {
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not open default emulators.json");
    }
    auto mappings_path = TKPEmu::EmulatorFactory::GetSavePath() + "mappings.json";
    QString data_mappings;
    if (std::filesystem::exists(mappings_path)) {
        QFile f2(mappings_path.c_str());
        if (!f2.open(QIODevice::ReadOnly)) {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not open mappings.json");
        }
        data_mappings = f2.readAll();
    } else {
        QFile f2(":/data/mappings.json");
        if (!f2.open(QIODevice::ReadOnly)) {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not open default mappings.json");
        }
        data_mappings = f2.readAll();
        std::ofstream ofs(mappings_path);
        if (ofs.is_open()) {
            ofs << data_mappings.toStdString();
            ofs.close();
        } else {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not open mappings.json");
        }
    }
    QString data = f.readAll();
    json j = json::parse(data.toStdString());
    json j_mappings = json::parse(data_mappings.toStdString());
    for (auto it = j.begin(); it != j.end(); ++it) {
        EmulatorData d;
        json& o = it.value();
        o.at("Name").get_to(d.Name);
        o.at("SettingsFile").get_to(d.SettingsFile);
        o.at("Extensions").get_to(d.Extensions);
        o.at("DefaultWidth").get_to(d.DefaultWidth);
        o.at("DefaultHeight").get_to(d.DefaultHeight);
        o.at("HasDebugger").get_to(d.HasDebugger);
        o.at("HasTracelogger").get_to(d.HasTracelogger);
        o.at("LoggingOptions").get_to(d.LoggingOptions);
        constant_map[std::stoi(it.key())] = d;
    }
    for (auto it = j_mappings.begin(); it != j_mappings.end(); ++it) {
        auto& d = constant_map[std::stoi(it.key())];
        json& o = it.value();
        o.at("KeyNames").get_to(d.Mappings.KeyNames);
        o.at("KeyValues").get_to(d.Mappings.KeyValues);
    }
    // Write default emulator options if they dont exist
    for (auto& e : constant_map) {
        if (!std::filesystem::exists(TKPEmu::EmulatorFactory::GetSavePath() + e.SettingsFile)) {
            std::ofstream ofs(TKPEmu::EmulatorFactory::GetSavePath() + e.SettingsFile);
            if (ofs.is_open()) {
                QFile resource(std::string(":/data/" + e.SettingsFile).c_str());
                if (!resource.open(QIODeviceBase::ReadOnly))
                    throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not open resource file");
                ofs << resource.readAll().toStdString();
                ofs.close();
            } else {
                throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not create default options file");
            }
        }
    }
    // Read emulator options
    for (int i = 0; i < static_cast<int>(TKPEmu::EmuType::EmuTypeSize); i++) {
        std::map<std::string, std::string> temp;
        const auto& e = constant_map[i];
        auto path = TKPEmu::EmulatorFactory::GetSavePath() + e.SettingsFile;
        std::ifstream ifs(path);
        if (ifs.is_open()) {
            std::stringstream buf;
            buf << ifs.rdbuf();
            json j = json::parse(buf.str());
            for (auto it = j.begin(); it != j.end(); ++it) {
                temp[it.key()] = it.value();
            }
        } else {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not open emulator options file");
        }
        EmulatorUserData user_data(path, temp);
        user_map[i] = std::move(user_data);
    }
    // Read general options
    {
        auto path = TKPEmu::EmulatorFactory::GetSavePath() + "settings.json";
        std::map<std::string, std::string> temp;
        if (std::filesystem::exists(path)) {
            std::ifstream ifs(path);
            if (ifs.is_open()) {
                std::stringstream buf;
                buf << ifs.rdbuf();
                json j = json::parse(buf.str());
                for (auto it = j.begin(); it != j.end(); ++it) {
                    temp[it.key()] = it.value();
                }
            } else {
                throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not open options file");
            }
        }
        GeneralSettings settings(path, temp);
        TKPEmu::EmulatorFactory::SetGeneralSettings(std::move(settings));
    }
    TKPEmu::EmulatorFactory::SetEmulatorData(std::move(constant_map));
    TKPEmu::EmulatorFactory::SetEmulatorUserData(std::move(user_map));
}

void MainWindow::pause_emulator() {
    if (emulator_->Paused.load()) {
        emulator_->Paused.store(false);
        emulator_->Step.store(true);
        emulator_->Step.notify_all();
    } else {
        message_queue_->PushRequest({
            .Id = RequestId::COMMON_PAUSE,
        });
    }
}

void MainWindow::reset_emulator() {
    empty_screen();
    message_queue_->PushRequest({
        .Id = RequestId::COMMON_RESET,
    });
}

void MainWindow::stop_emulator() {
    if (emulator_) {
        empty_screen();
        emulator_->CloseAndWait();
        emulator_.reset();
        enable_emulation_actions(false);
    }
}

void MainWindow::redraw_screen() {
    if (!emulator_)
        return;
    std::lock_guard<std::mutex> lg(emulator_->DrawMutex);
    if (!emulator_->IsReadyToDraw())
        return;
    screen_->Redraw(emulator_->GetWidth(), emulator_->GetHeight(), emulator_->GetBitdepth(), emulator_->GetScreenData());
    screen_->update();
    emulator_->IsReadyToDraw() = false;
}

void MainWindow::empty_screen() {
    std::vector<uint8_t> empty_screen;
    empty_screen.resize(emulator_->GetWidth() * emulator_->GetHeight() * 4);
    std::fill(empty_screen.begin(), empty_screen.end(), 0);
    screen_->Redraw(emulator_->GetWidth(), emulator_->GetHeight(), emulator_->GetBitdepth(), empty_screen.data());
    screen_->update();
}