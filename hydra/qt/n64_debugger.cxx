#include "n64_debugger.hxx"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QTimer>
#include <iostream>
#include <string>
#include <utility>
#include <fmt/format.h>

std::string N64Debugger::get_gpr_value(int n) {
    std::shared_lock lock(emulator_->DataMutex);
    return fmt::format("0x{:016x}", emulator_->n64_impl_.cpu_.gpr_regs_[n].UD);
}

std::string N64Debugger::get_gpr_name(int n) {
    if (register_names_) {
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
        default:
            std::unreachable();
        }
    } else {
        return "r" + std::to_string(n);
    }
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

    QTimer *timer = new QTimer(this);
    timer->start(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_debugger_tab()));
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

void N64Debugger::on_tab_change() {
    tab_show_->setCurrentIndex(tab_list_->currentRow());
}

void N64Debugger::SetEmulator(TKPEmu::N64::N64_TKPWrapper* emulator) {
    emulator_ = emulator;
}

void N64Debugger::update_debugger_tab() {
    std::shared_lock lock(emulator_->DataMutex);
    if (!emulator_->Paused || !was_paused_) {
        was_paused_ = emulator_->Paused;
        switch (tab_list_->currentRow()) {
        case 0:
            for (int i = 0; i < 32; i++) {
                gpr_edit_[i]->blockSignals(true);
                gpr_edit_[i]->setText(QString::fromStdString(get_gpr_value(i)));
                gpr_edit_[i]->blockSignals(false);
                gpr_edit_[i]->setReadOnly(!emulator_->Paused);
            }
            break;
        }
    }
}

void N64Debugger::create_tabs() {
    #define X(name) QListWidgetItem* name = new QListWidgetItem(#name); tab_list_->addItem(name); name##_tab = new QWidget; tab_show_->addTab(name##_tab, #name); name##_layout = new QGridLayout; name##_tab->setLayout(name##_layout);
    N64_DEBUGGER_TABS
    #undef X
    create_registers_tab();
    create_settings_tab();
    tab_list_->setCurrentItem(Registers);
    tab_show_->setCurrentIndex(0);
}

void N64Debugger::register_changed(const QString&, int reg) {
    std::unique_lock lock(emulator_->DataMutex);
    bool ok = false;
    auto old = emulator_->n64_impl_.cpu_.gpr_regs_[reg].UD;
    emulator_->n64_impl_.cpu_.gpr_regs_[reg].UD = gpr_edit_[reg]->text().toULongLong(&ok, 16);
    if (!ok) {
        emulator_->n64_impl_.cpu_.gpr_regs_[reg].UD = old;
    }
}

void N64Debugger::create_registers_tab() {
    Registers_layout->addWidget(new QLabel("Registers"), 0, 0, 1, 1);
    QFrame *line;
    line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    Registers_layout->addWidget(line, 1, 0, 1, 2);
    for (int i = 0; i < 32; i++) {
        Registers_layout->addWidget(new QLabel(QString::fromStdString(get_gpr_name(i))), i + 2, 0, 1, 1);
        QLineEdit*& text = gpr_edit_[i];
        text = new QLineEdit;
        text->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
        text->setInputMask("\\0\\xHHHHHHHHHHHHHHHH");
        text->setText("0x0000000000000000");
        text->setReadOnly(true);
        text->setFont(fixedfont);
        connect(text, &QLineEdit::textChanged, this, std::bind(&N64Debugger::register_changed, this, std::placeholders::_1, i));
        Registers_layout->addWidget(text, i + 2, 1, 1, 1);
    }
}

void N64Debugger::create_settings_tab() {

}