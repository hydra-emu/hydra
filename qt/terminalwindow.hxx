#pragma once

#include <QComboBox>
#include <QTextEdit>
#include <QWidget>
#include <unordered_map>

class TerminalWindow : public QWidget
{
    Q_OBJECT

public:
    TerminalWindow(bool& open, QWidget* parent = nullptr);
    ~TerminalWindow();

    static void log(const char* group, const char* message);
    static void log_warn(const char* message);
    static void log_info(const char* message);
    static void log_debug(const char* message);

private:
    QComboBox* groups_combo_box_;
    QTextEdit* edit_;
    bool& open_;

    void on_group_changed(const QString& group);
    void on_timeout();

    static std::unordered_map<std::string, std::string> logs_;
    static bool changed_;
};