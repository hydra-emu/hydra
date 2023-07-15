#ifndef SHADEREDITOR_H
#define SHADEREDITOR_H
#include <functional>
#include <QFile>
#include <QList>
#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QToolBar>
#include <QWidget>

class ShaderHighlighter final : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    ShaderHighlighter(QTextDocument* parent = nullptr);

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void highlightBlock(const QString& text) override;
    QRegularExpression comment_start_expression_;
    QRegularExpression comment_end_expression_;
    QTextCharFormat multiline_comment_format_;
    QTextCharFormat singleline_comment_format_;
    QTextCharFormat hashtag_format_;
    QList<HighlightingRule> highlighting_rules_;
    QTextCharFormat keyword_blue_format_;
    QTextCharFormat keyword_pink_format_;
};

class ShaderEditor final : public QWidget
{
    Q_OBJECT

public:
    ShaderEditor(bool& open, std::function<void(QString*, QString*)> callback,
                 QWidget* parent = nullptr);
    ~ShaderEditor();
    ShaderEditor(const ShaderEditor&) = delete;
    ShaderEditor& operator=(const ShaderEditor&) = delete;
private slots:
    void compile();
    void autocompile();
    void open_shader();

private:
    QTextEdit* editor_;
    QToolBar* toolbar_;
    QAction* compile_act_;
    QAction* autocompile_act_;
    QAction* open_act_;
    ShaderHighlighter* highlighter_;
    std::function<void(QString*, QString*)> callback_;
    bool& open_;
    bool autocompile_ = false;
};
#endif