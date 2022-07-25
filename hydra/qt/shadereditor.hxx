#ifndef SHADEREDITOR_H
#define SHADEREDITOR_H
#include <QWidget>
#include <QSyntaxHighlighter>
#include <QList>
#include <QString>
#include <QTextCharFormat>
#include <QRegularExpression>

class ShaderHighlighter final : public QSyntaxHighlighter {
    Q_OBJECT
public:
    ShaderHighlighter(QTextDocument *parent = nullptr);
private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    void highlightBlock(const QString& text) override;
    QList<HighlightingRule> highlighting_rules_;
    QTextCharFormat keyword_format_;
};

class ShaderEditor final : public QWidget {
    Q_OBJECT
public:
    ShaderEditor(bool& open, QWidget* parent = nullptr);
    ~ShaderEditor();
    ShaderEditor(const ShaderEditor&) = delete;
    ShaderEditor& operator=(const ShaderEditor&) = delete;
private:
    bool& open_;
};
#endif