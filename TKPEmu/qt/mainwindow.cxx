#include "mainwindow.hxx"
#include "../include/error_factory.hxx"
#include <QMessageBox>
#include <QTimer>
#include <iostream>

#define QT_MAY_THROW(func) try {\
    func \
} catch (std::exception& ex) { \
    QMessageBox messageBox; \
    messageBox.critical(0,"Error", ex.what()); \
    messageBox.setFixedSize(500,200); \
    messageBox.exec(); \
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    if(SDL_Init(SDL_INIT_AUDIO) != 0) [[unlikely]] {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        exit(1);
    }
    std::cout << "SDL initialized successfully" << std::endl;
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
}

void MainWindow::create_actions() {
    open_act_ = new QAction(tr("&Open ROM"), this);
    open_act_->setShortcuts(QKeySequence::Open);
    open_act_->setStatusTip(tr("Open a ROM"));
    connect(open_act_, &QAction::triggered, this, &MainWindow::open_file);
    pause_act_ = new QAction(tr("&Pause"), this);
    pause_act_->setShortcut(Qt::CTRL | Qt::Key_P);
    pause_act_->setStatusTip(tr("Pause emulation"));
    connect(pause_act_, &QAction::triggered, this, &MainWindow::pause_emulator);
}

void MainWindow::create_menus() {
    file_menu_ = menuBar()->addMenu(tr("&File"));
    file_menu_->addAction(open_act_);
    emulation_menu_ = menuBar()->addMenu(tr("&Emulation"));
    emulation_menu_->addAction(pause_act_);
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
        emulator_->SkipBoot = true;
        auto func = [&]() {
            emulator_->Start();
        };
        emulator_thread_ = std::thread(func);
        emulator_thread_.detach();
    );
}

void MainWindow::pause_emulator() {
    
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

MainWindow::~MainWindow()
{
}

