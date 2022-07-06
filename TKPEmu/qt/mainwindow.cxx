#include "mainwindow.hxx"
#include "settingswindow.hxx"
#include "../include/error_factory.hxx"
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <iostream>

#define QT_MAY_THROW(func) try {\
    func \
} catch (std::exception& ex) { \
    QMessageBox messageBox; \
    messageBox.critical(0,"Error", ex.what()); \
    messageBox.setFixedSize(500,200); \
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setup_emulator_specific();
    if(SDL_Init(SDL_INIT_AUDIO) != 0) [[unlikely]] {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        exit(1);
    }
    QWidget *widget = new QWidget;
    setCentralWidget(widget);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(5, 5, 5, 5);
    lbl_ = new QLabel(this);
    lbl_->setScaledContents(false);
    lbl_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lbl_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(lbl_);
    widget->setLayout(layout);
    create_actions();
    create_menus();
    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);
    setMinimumSize(160, 160);
    resize(480, 320);

    QTimer *timer = new QTimer;
    timer->start(16);
    connect(timer, SIGNAL(timeout()), this, SLOT(redraw_screen()));

    enable_emulation_actions(false);
}

void MainWindow::create_actions() {
    open_act_ = new QAction(tr("&Open ROM"), this);
    open_act_->setShortcuts(QKeySequence::Open);
    open_act_->setStatusTip(tr("Open a ROM"));
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
    screenshot_act_ = new QAction(tr("S&creenshot to clipboard"), this);
    screenshot_act_->setShortcut(Qt::CTRL | Qt::Key_C);
    screenshot_act_->setStatusTip(tr("Take a screenshot to clipboard"));
    connect(screenshot_act_, &QAction::triggered, this, &MainWindow::screenshot);
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
    std::string path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(""
        "All supported types (*.gb *.gbc *.nes *.ch8 *.z64);;"
        "Gameboy (*.gb *.gbc);;"
        "NES (*.nes);;"
        "Chip 8 (*.ch8);;"
        "Nintendo 64 (*.z64);;"
    )).toStdString();
    if (path.empty())
        return;
    QT_MAY_THROW(
        auto type = TKPEmu::EmulatorFactory::GetEmulatorType(path);
        {
            auto emulator = TKPEmu::EmulatorFactory::Create(type);
            std::swap(emulator, emulator_);
            // Old emulator is destroyed here
        }
        if (!emulator_->LoadFromFile(path))
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to open ROM");
        message_queue_ = emulator_->MessageQueue;
        emulator_->SkipBoot = true;
        auto func = [&]() {
            emulator_->Start();
        };
        emulator_thread_ = std::thread(func);
        emulator_thread_.detach();
        enable_emulation_actions(true);
    );
}

void MainWindow::open_settings() {
    if (!settings_open_) {
        QWidget* qw = new SettingsWindow(settings_open_);
        qw->show();
    }
}

void MainWindow::screenshot() {
    // QApplication::clipboard()->setImage(texture_.toImage());
}

void MainWindow::enable_emulation_actions(bool should) {
    pause_act_->setEnabled(should);
    stop_act_->setEnabled(should);
    reset_act_->setEnabled(should);
    lbl_->setVisible(should);
    if (!should) {
        pause_act_->setChecked(false);
    }
}

void MainWindow::setup_emulator_specific() {
    QFile f(":/data/matches.json");
    if (!f.open(QIODevice::ReadOnly)) {
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not open matches.json");
    } else {
    }
    QString data = f.readAll();
    json j = json::parse(data.toStdString());
    EmulatorDataMap map;
    for (auto it = j.begin(); it != j.end(); ++it) {
        EmulatorData d;
        json& o = it.value();
        o.at("Name").get_to(d.Name);
        o.at("SettingsFile").get_to(d.SettingsFile);
        o.at("Extensions").get_to(d.Extensions);
        map[std::stoi(it.key())] = d;
    }
    // Setup default options
    for (auto& e : map) {
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

    TKPEmu::EmulatorFactory::SetEmulatorData(map);
}

void MainWindow::pause_emulator() {
    if (emulator_->Paused.load()) {
        emulator_->Paused.store(false);
        emulator_->Step.store(true);
        emulator_->Step.notify_all();
    } else {
        message_queue_->PushRequest("pause");
    }
}

void MainWindow::reset_emulator() {
    message_queue_->PushRequest("reset");
}

void MainWindow::stop_emulator() {
    emulator_->CloseAndWait();
    emulator_.reset();
    enable_emulation_actions(false);
}

void MainWindow::redraw_screen() {
    if (!emulator_)
        return;
    std::lock_guard<std::mutex> lg(emulator_->DrawMutex);
    if (!emulator_->IsReadyToDraw())
        return;
    QImage image((const unsigned char*)emulator_->GetScreenData(), 160, 144, QImage::Format_RGBA8888);
    lbl_->setPixmap(QPixmap::fromImage(image.scaled(lbl_->width(), lbl_->height(), Qt::KeepAspectRatio, Qt::FastTransformation)));
}
