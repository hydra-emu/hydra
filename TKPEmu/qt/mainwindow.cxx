#include "mainwindow.hxx"
// #include "../include/emulator.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *widget = new QWidget;
    setCentralWidget(widget);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    lbl_ = new QLabel(this);
    lbl_->setScaledContents(true);
    lbl_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    layout->addWidget(lbl_);
    widget->setLayout(layout);
    create_actions();
    create_menus();
    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);
    setWindowTitle(tr("Menus"));
    setMinimumSize(160, 160);
    resize(480, 320);
}

void MainWindow::create_actions() {
    open_act_ = new QAction(tr("&Open ROM"), this);
    open_act_->setShortcuts(QKeySequence::Open);
    open_act_->setStatusTip(tr("Open a ROM"));
    connect(open_act_, &QAction::triggered, this, &MainWindow::open_file);
    pause_act_ = new QAction(tr("&Pause"), this);
    pause_act_->setShortcut(Qt::CTRL | Qt::Key_P);
    open_act_->setStatusTip(tr("Pause emulation"));
    connect(open_act_, &QAction::triggered, this, &MainWindow::pause_emulator);
}

void MainWindow::create_menus() {
    file_menu_ = menuBar()->addMenu(tr("&File"));
    file_menu_->addAction(open_act_);
    emulation_menu_ = menuBar()->addMenu(tr("&Emulation"));
    emulation_menu_->addAction(pause_act_);
}

void MainWindow::open_file() {
    auto path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(""
        "All supported types (*.gb *.gbc *.nes *.ch8 *.z64);;"
        "Gameboy (*.gb *.gbc);;"
        "NES (*.nes);;"
        "Chip 8 (*.ch8);;"
        "Nintendo 64 (*.z64);;"
    ));
    auto type = TKPEmu::EmulatorFactory::GetEmulatorType(path.toStdString());
    auto emulator = TKPEmu::EmulatorFactory::Create(type);
}

void MainWindow::pause_emulator() {

}

MainWindow::~MainWindow()
{
}

