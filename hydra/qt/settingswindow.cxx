#include "settingswindow.hxx"
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QPushButton>
#include <QScrollBar>
#include <QCheckBox>
#include <include/emulator_types.hxx>
#include <include/emulator_factory.h>
#define emu_data(emu_type) TKPEmu::EmulatorFactory::GetEmulatorUserData()[static_cast<int>(emu_type)]

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
        right_group_box_->setMinimumWidth(500);
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
        auto dmg_path = emu_data(TKPEmu::EmuType::Gameboy).Get("dmg_path");
        auto cgb_path = emu_data(TKPEmu::EmuType::Gameboy).Get("cgb_path");
        auto skip_bios_val = emu_data(TKPEmu::EmuType::Gameboy).Get("skip_bios") == "true";
        QGridLayout* gb_layout = new QGridLayout;
        dmg_bios_path_ = new QLineEdit;
        dmg_bios_path_->setReadOnly(true);
        dmg_bios_path_->setText(dmg_path.c_str());
        cgb_bios_path_ = new QLineEdit;
        cgb_bios_path_->setReadOnly(true);
        cgb_bios_path_->setText(cgb_path.c_str());
        QPushButton* dmg_file_pick = new QPushButton;
        dmg_file_pick->setText("...");
        connect(dmg_file_pick, SIGNAL(clicked()), this, SLOT(on_dmg_click()));
        QPushButton* cgb_file_pick = new QPushButton;
        cgb_file_pick->setText("...");
        connect(cgb_file_pick, SIGNAL(clicked()), this, SLOT(on_cgb_click()));
        gb_layout->setColumnStretch(1, 3);
        gb_layout->addWidget(new QLabel("DMG bios path:"), 0, 0);
        gb_layout->addWidget(dmg_bios_path_, 0, 1);
        gb_layout->addWidget(dmg_file_pick, 0, 2);
        gb_layout->addWidget(new QLabel("CGB bios path:"), 1, 0);
        gb_layout->addWidget(cgb_bios_path_, 1, 1);
        gb_layout->addWidget(cgb_file_pick, 1, 2);
        QCheckBox* skip_bios = new QCheckBox("Skip bios?");
        skip_bios->setChecked(skip_bios_val);
        connect(skip_bios, SIGNAL(stateChanged()), this, SLOT(on_gb_skip_bios_click()));
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

void SettingsWindow::on_dmg_click() {
    auto path = QFileDialog::getOpenFileName(this, tr("Open ROM"), "", "Binary files (*.bin)");
    emu_data(TKPEmu::EmuType::Gameboy).Set("dmg_path", path.toStdString());
    emu_data(TKPEmu::EmuType::Gameboy).Save();
    dmg_bios_path_->setText(path);
}

void SettingsWindow::on_cgb_click() {
    auto path = QFileDialog::getOpenFileName(this, tr("Open ROM"), "", "Binary files (*.bin)");
    emu_data(TKPEmu::EmuType::Gameboy).Set("cgb_path", path.toStdString());
    emu_data(TKPEmu::EmuType::Gameboy).Save();
    cgb_bios_path_->setText(path);
}

void SettingsWindow::on_gb_skip_bios_click(int state) {
    auto str = (state == Qt::CheckState::Checked) ? "true" : "false";
    emu_data(TKPEmu::EmuType::Gameboy).Set("skip_bios", str);
    emu_data(TKPEmu::EmuType::Gameboy).Save();
}

void SettingsWindow::on_tab_change() {
    tab_show_->setCurrentIndex(tab_list_->currentRow());
}