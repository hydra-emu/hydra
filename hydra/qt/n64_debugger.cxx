#include "n64_debugger.hxx"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <iostream>

N64Debugger::N64Debugger(bool& open, std::shared_ptr<TKPEmu::Tools::MQBase> mq, QWidget* parent)
    : open_(open),
    message_queue_(mq),
    emulator_type_(TKPEmu::EmuType::N64),
    QWidget(parent, Qt::Window)
{
    QVBoxLayout* layout = new QVBoxLayout;
    QGroupBox* top_qgb = new QGroupBox;
    {
        QVBoxLayout* top_layout = new QVBoxLayout;
        {
            pc_label_ = new QLabel;
            QPushButton* button = new QPushButton("Test");
            top_layout->addWidget(pc_label_);
            text_edit_ = new QTextEdit;
            top_layout->addWidget(text_edit_);
            connect(button, SIGNAL(clicked()), this, SLOT(test()));
            QPushButton* button2 = new QPushButton("Test2");
            connect(button2, SIGNAL(clicked()), this, SLOT(test2()));
            QPushButton* button3 = new QPushButton("Pi DMA");
            connect(button3, SIGNAL(clicked()), this, SLOT(pi_dma()));
            QPushButton* button4 = new QPushButton("Si DMA");
            connect(button4, SIGNAL(clicked()), this, SLOT(si_dma()));
            QPushButton* button5 = new QPushButton("Vi DMA");
            connect(button5, SIGNAL(clicked()), this, SLOT(vi_dma()));
            top_layout->addWidget(button);
            top_layout->addWidget(button2);
            top_layout->addWidget(button3);
            top_layout->addWidget(button4);
            top_layout->addWidget(button5);
        }
        top_qgb->setLayout(top_layout);
    }
    layout->addWidget(top_qgb);
    setLayout(layout);
    setWindowTitle("Debugger");
    show();
    open_ = true;
}

N64Debugger::~N64Debugger() {}

void N64Debugger::test() {
    std::stringstream ss;
    ss << "PC: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.pc_ << '\n';
    ss << "$zero: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[0].UD << '\n';
    ss << "$at: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[1].UD << '\n';
    ss << "$v0: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[2].UD << '\n';
    ss << "$v1: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[3].UD << '\n';
    for (int i = 0; i < 4; i++) {
        ss << "$a" << i << ": " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[4 + i].UD << '\n';
    }
    for (int i = 0; i < 8; i++) {
        ss << "$t" << i << ": " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[8 + i].UD << '\n';
    }
    for (int i = 0; i < 8; i++) {
        ss << "$s" << i << ": " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[16 + i].UD << '\n';
    }
    ss << "$t8: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[24].UD << '\n';
    ss << "$t9: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[25].UD << '\n';
    ss << "$k0: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[26].UD << '\n';
    ss << "$k1: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[27].UD << '\n';
    ss << "$gp: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[28].UD << '\n';
    ss << "$sp: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[29].UD << '\n';
    ss << "$fp: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[30].UD << '\n';
    ss << "$ra: " << std::hex << std::setfill('0') << std::setw(8) << emulator_->n64_impl_.cpu_.gpr_regs_[31].UD << '\n';
    QString text = QString::fromStdString(ss.str());
    pc_label_->setText(text);
}

void N64Debugger::test2() {
    uint8_t* data = emulator_->n64_impl_.cpubus_.redirect_paddress(0x0000'0180);
    std::stringstream ss;
    for (int i = 0; i < 10000; i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)data[i];
    }
    text_edit_->setText(QString::fromStdString(ss.str()));
}

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