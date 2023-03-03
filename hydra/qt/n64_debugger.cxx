#include "n64_debugger.hxx"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QListWidgetItem>
#include <iostream>
#include <string>
#include <utility>

bool registerNames = false;

std::string get_gpr_name(int n) {
    if (registerNames) {
        switch (n) {
        case 0:
            return "zero";
        case 1:
            return "at";
        case 2 ... 3:
            return "v" + std::to_string(n - 2);
        case 4 ... 7:
            return "a" + std::to_string(n - 4);
        case 8 ... 15:
            return "t" + std::to_string(n - 8);
        case 16 ... 23:
            return "s" + std::to_string(n - 16);
        case 24 ... 25:
            return "t" + std::to_string(n - 16);
        case 26 ... 27:
            return "k" + std::to_string(n - 26);
        case 28:
            return "gp";
        case 29:
            return "sp";
        case 30:
            return "fp";
        case 31:
            return "ra";
        }
    } else {
        return "r" + std::to_string(n);
    }
    std::unreachable();
}

N64Debugger::N64Debugger(bool& open, QWidget* parent)
    : open_(open),
    emulator_type_(TKPEmu::EmuType::N64),
    QWidget(parent, Qt::Window)
{
    QGridLayout* main_layout = new QGridLayout;
    left_group_box_ = new QGroupBox;
    right_group_box_ = new QGroupBox;
    {
        tab_list_ = new QListWidget;
        QVBoxLayout* left_layout = new QVBoxLayout;
        left_layout->addWidget(tab_list_);
        left_group_box_->setLayout(left_layout);
        left_group_box_->setFixedWidth(200);
        left_group_box_->setMinimumHeight(400);
        connect(tab_list_, SIGNAL(itemSelectionChanged()), this, SLOT(on_tab_change()));
    }
    {
        tab_show_ = new QTabWidget;
        tab_show_->tabBar()->hide();
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(5, 5, 5, 5);
        layout->addWidget(tab_show_);
        right_group_box_->setLayout(layout);
        right_group_box_->setMinimumWidth(500);
        right_group_box_->setMinimumHeight(400);
    }
    create_tabs();
    main_layout->addWidget(left_group_box_, 0, 0, 1, 1);
    main_layout->addWidget(right_group_box_, 0, 1, 1, 5);
    main_layout->setColumnStretch(0, 0);
    main_layout->setColumnStretch(1, 1);
    setLayout(main_layout);
    setWindowTitle("Debugger");
    show();
    open_ = true;
}

N64Debugger::~N64Debugger() {}

void N64Debugger::pi_dma() {
    emulator_->n64_impl_.cpu_.queue_event(SchedulerEventType::Pi, 100);
}

void N64Debugger::si_dma() {
    emulator_->n64_impl_.cpu_.queue_event(SchedulerEventType::Si, 100);
}

void N64Debugger::vi_dma() {
    emulator_->n64_impl_.cpu_.queue_event(SchedulerEventType::Vi, 100);
}

void N64Debugger::SetEmulator(TKPEmu::N64::N64_TKPWrapper* emulator) {
    emulator_ = emulator;
}

void N64Debugger::customEvent(QEvent* event) {
    std::shared_lock lock(emulator_->DataMutex);
    
}

void N64Debugger::create_tabs() {
    #define add_tab(name) QListWidgetItem* name = new QListWidgetItem(#name); tab_list_->addItem(name); QWidget* name##_tab = new QWidget; tab_show_->addTab(name##_tab, #name);
    add_tab(Registers)
    #undef add_tab
    tab_list_->setCurrentItem(Registers);
    tab_show_->setCurrentIndex(0);
}