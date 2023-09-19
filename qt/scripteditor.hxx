#pragma once

#include <QCheckBox>
#include <QList>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QToolBar>
#include <QWidget>

class ScriptHighlighter final : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    ScriptHighlighter(QTextDocument* parent = nullptr);

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void highlightBlock(const QString& text) override;

    QList<HighlightingRule> highlighting_rules_;
};

class ScriptEditor final : public QWidget
{
    Q_OBJECT

public:
    ScriptEditor(bool& open, std::function<void(const std::string&, bool)> run_script_callback,
                 QWidget* parent = nullptr);
    ~ScriptEditor();

private:
    QTextEdit* editor_;
    QToolBar* toolbar_;
    QAction* run_act_;
    QCheckBox* priviledged_mode_;
    ScriptHighlighter* highlighter_;
    std::function<void(const std::string&, bool)> run_script_callback_;
    bool& open_;
    bool safe_mode_ = true;

    void run_script();
    void open_script();
    void save_script();
    void safe_mode_changed(int state);

    ScriptEditor(const ScriptEditor&) = delete;
    ScriptEditor& operator=(const ScriptEditor&) = delete;
};