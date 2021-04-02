#pragma once
#ifndef TKP_TRACELOGGER_H
#define TKP_TRACELOGGER_H
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QComboBox>
#include <lib/messagequeue.hxx>
#include <include/emulator_types.hxx>

class TraceloggerWindow : public QWidget {
    Q_OBJECT
private:
    bool& open_;
    std::shared_ptr<TKPEmu::Tools::MQBase> message_queue_;
    TKPEmu::EmuType emulator_type_;
    QComboBox* combo_box_;
    std::vector<QListWidgetItem*> checkboxes_;
    QString log_path_;
    QPlainTextEdit* text_edit_;
    QPushButton* log_button_;
    bool is_logging_ = false;
private slots:
    void browse_clicked();
    void log_clicked();
public:
    TraceloggerWindow(bool& open, std::shared_ptr<TKPEmu::Tools::MQBase> mq, TKPEmu::EmuType type, QWidget* parent = nullptr);
    ~TraceloggerWindow();
    void Reset(std::shared_ptr<TKPEmu::Tools::MQBase> mq, TKPEmu::EmuType type);
    static int GetToolIndex() {
        return 1;
    }
};
#endif