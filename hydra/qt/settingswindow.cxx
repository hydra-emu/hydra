#include "settingswindow.hxx"
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QCheckBox>

SettingsWindow::SettingsWindow(bool& open, QWidget* parent) : open_(open), QWidget(parent, Qt::Window) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Settings");
    QGridLayout* main_layout = new QGridLayout;
    left_group_box_ = new QGroupBox;
    right_group_box_ = new QGroupBox;
    {
        tab_list_ = new QListWidget;
        QListWidgetItem* general = new QListWidgetItem(QPixmap(":/images/support.png"), QString("General"));
        QListWidgetItem* input = new QListWidgetItem(QPixmap(":/images/input.png"), QString("Input"));
        QListWidgetItem* gameboy = new QListWidgetItem(QPixmap(":/images/gameboy.png"), QString("Gameboy"));
        QListWidgetItem* n64 = new QListWidgetItem(QPixmap(":/images/n64.png"), QString("Nintendo 64"));
        QListWidgetItem* chip8 = new QListWidgetItem(QPixmap(":/images/chip8.png"), QString("Chip 8"));
        tab_list_->addItem(general);
        tab_list_->addItem(input);
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
        QGridLayout* input_layout = new QGridLayout;
        QWidget* input_tab = new QWidget;
        input_tab->setLayout(input_layout);
        tab_show_->addTab(input_tab, "Input");
    }
    {
        QGridLayout* gb_layout = new QGridLayout;
        QPlainTextEdit* dmg_bios_path = new QPlainTextEdit;
        QFontMetrics m(dmg_bios_path->font());
        int height = m.lineSpacing();
        dmg_bios_path->setReadOnly(true);
        dmg_bios_path->verticalScrollBar()->hide();
        QPlainTextEdit* cgb_bios_path = new QPlainTextEdit;
        cgb_bios_path->setReadOnly(true);
        cgb_bios_path->setFixedHeight(height * 1.5);
        cgb_bios_path->verticalScrollBar()->hide();
        QPushButton* dmg_file_pick = new QPushButton;
        dmg_file_pick->setText("...");
        dmg_bios_path->setFixedHeight(height * 1.5);
        QPushButton* cgb_file_pick = new QPushButton;
        cgb_file_pick->setText("...");
        gb_layout->addWidget(new QLabel("DMG bios path:"), 0, 0);
        gb_layout->addWidget(dmg_bios_path, 0, 1);
        gb_layout->addWidget(dmg_file_pick, 0, 2);
        gb_layout->addWidget(new QLabel("CGB bios path:"), 1, 0);
        gb_layout->addWidget(cgb_bios_path, 1, 1);
        gb_layout->addWidget(cgb_file_pick, 1, 2);
        QCheckBox* skip_bios = new QCheckBox("Skip bios?");
        gb_layout->addWidget(skip_bios, 2, 0);
        QWidget* empty = new QWidget();
        empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        gb_layout->addWidget(empty, 10, 0);
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