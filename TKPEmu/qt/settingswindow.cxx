#include "settingswindow.hxx"
#include <QVBoxLayout>
#include <QGroupBox>

SettingsWindow::SettingsWindow(bool& open, QWidget* parent) : open_(open), QWidget(parent, Qt::Window) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Settings");
    QGridLayout* main_layout = new QGridLayout;
    QGroupBox* left_group_box = new QGroupBox;
    QGroupBox* right_group_box = new QGroupBox;
    {
        QListWidget* tab_list = new QListWidget;
        QListWidgetItem* general = new QListWidgetItem(QPixmap(":/images/support.png"), QString("General"));
        QListWidgetItem* gameboy = new QListWidgetItem(QPixmap(":/images/gameboy.png"), QString("Gameboy"));
        QListWidgetItem* n64 = new QListWidgetItem(QPixmap(":/images/n64.png"), QString("Nintendo 64"));
        QListWidgetItem* chip8 = new QListWidgetItem(QPixmap(":/images/chip8.png"), QString("Chip 8"));
        tab_list->addItem(general);
        tab_list->addItem(chip8);
        tab_list->addItem(gameboy);
        tab_list->addItem(n64);
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(tab_list);
        left_group_box->setLayout(layout);
    }
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(5, 5, 5, 5);
        right_group_box->setLayout(layout);
    }
    main_layout->addWidget(left_group_box, 0, 0, 1, 1);
    main_layout->addWidget(right_group_box, 0, 1, 1, 5);
    main_layout->setColumnStretch(0, 0);
    main_layout->setColumnStretch(1, 1);
    setLayout(main_layout);
    show();
    open_ = true;
}

SettingsWindow::~SettingsWindow() {
    open_ = false;
}