#include "settingswindow.hxx"
#include <iostream>
#include <QVBoxLayout>
#include <QLabel>

SettingsWindow::SettingsWindow(bool& open, QWidget* parent) : open_(open), QWidget(parent, Qt::Window) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Settings");
    QGridLayout* main_layout = new QGridLayout;
    left_group_box_ = new QGroupBox;
    right_group_box_ = new QGroupBox;
    {
        tab_list_ = new QListWidget;
        QListWidgetItem* general = new QListWidgetItem(QPixmap(":/images/support.png"), QString("General"));
        QListWidgetItem* gameboy = new QListWidgetItem(QPixmap(":/images/gameboy.png"), QString("Gameboy"));
        QListWidgetItem* n64 = new QListWidgetItem(QPixmap(":/images/n64.png"), QString("Nintendo 64"));
        QListWidgetItem* chip8 = new QListWidgetItem(QPixmap(":/images/chip8.png"), QString("Chip 8"));
        tab_list_->addItem(general);
        tab_list_->addItem(gameboy);
        tab_list_->addItem(n64);
        tab_list_->addItem(chip8);
        connect(tab_list_, SIGNAL(itemSelectionChanged()), this, SLOT(on_tab_change()));
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(tab_list_);
        left_group_box_->setLayout(layout);
        left_group_box_->setFixedWidth(200);
        left_group_box_->setMinimumHeight(400);
    }
    {
        tab_show_ = new QTabWidget;
        tab_show_->tabBar()->hide();
        create_tabs();
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(5, 5, 5, 5);
        layout->addWidget(tab_show_);
        right_group_box_->setLayout(layout);
        right_group_box_->setMinimumWidth(400);
        right_group_box_->setMinimumHeight(400);
    }
    main_layout->addWidget(left_group_box_, 0, 0, 1, 1);
    main_layout->addWidget(right_group_box_, 0, 1, 1, 5);
    main_layout->setColumnStretch(0, 0);
    main_layout->setColumnStretch(1, 1);
    setLayout(main_layout);
    show();
    open_ = true;
}

SettingsWindow::~SettingsWindow() {
    open_ = false;
}

void SettingsWindow::create_tabs() {
    {
        QGridLayout* general_layout = new QGridLayout;
        QWidget* general_tab = new QWidget;
        general_tab->setLayout(general_layout);
        tab_show_->addTab(general_tab, "General");
    }
    {
        QGridLayout* gb_layout = new QGridLayout;
        gb_layout->addWidget(new QLabel("Test"));
        QWidget* gb_tab = new QWidget;
        gb_tab->setLayout(gb_layout);
        tab_show_->addTab(gb_tab, "GB");
    }
    {
    QGridLayout* nes_layout = new QGridLayout;
    QWidget* nes_tab = new QWidget;
    nes_tab->setLayout(nes_layout);
    tab_show_->addTab(nes_tab, "NES");
    }
    {
        QGridLayout* n64_layout = new QGridLayout;
        QWidget* n64_tab = new QWidget;
        n64_tab->setLayout(n64_layout);
        tab_show_->addTab(n64_tab, "N64");
    }
    {
        QGridLayout* c8_layout = new QGridLayout;
        QWidget* c8_tab = new QWidget;
        c8_tab->setLayout(c8_layout);
        tab_show_->addTab(c8_tab, "C8");
    }
}

void SettingsWindow::on_tab_change() {
    tab_show_->setCurrentIndex(tab_list_->currentRow());
}