#pragma once
#ifndef TKP_DEBUGGER_H
#define TKP_DEBUGGER_H
#include <QWidget>
#include <QComboBox>
#include <lib/messagequeue.hxx>

class DebuggerWindow : public QWidget {
    Q_OBJECT
private:
    bool& open_;
    std::shared_ptr<TKPEmu::Tools::MQBase> message_queue_;
    QComboBox* combo_box_;
public:
    DebuggerWindow(bool& open, std::shared_ptr<TKPEmu::Tools::MQBase> mq, QWidget* parent = nullptr);
    ~DebuggerWindow();
    static int GetToolIndex() {
        return 0;
    }
};
#endif