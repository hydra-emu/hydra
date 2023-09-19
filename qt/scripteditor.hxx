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
    ScriptEditor(bool& open, std::function<void(const std::string&)> run_script_callback,
                 QWidget* parent = nullptr);
    ~ScriptEditor();

private slots:
    void run_script();

private:
    QTextEdit* editor_;
    QToolBar* toolbar_;
    QAction* run_act_;
    QCheckBox* priviledged_mode_;
    ScriptHighlighter* highlighter_;
    std::function<void(const std::string&)> run_script_callback_;
    bool& open_;

    ScriptEditor(const ScriptEditor&) = delete;
    ScriptEditor& operator=(const ScriptEditor&) = delete;
};