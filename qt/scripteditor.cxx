#include "scripteditor.hxx"

#include <QVBoxLayout>

ScriptHighlighter::ScriptHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
{
    const QString patterns_pink[] = {
        QStringLiteral("\\bif\\b"),     QStringLiteral("\\belse\\b"),
        QStringLiteral("\\belseif\\b"), QStringLiteral("\\bfor\\b"),
        QStringLiteral("\\bwhile\\b"),  QStringLiteral("\\bdo\\b"),
        QStringLiteral("\\bend\\b"),    QStringLiteral("\\brepeat\\b"),
        QStringLiteral("\\buntil\\b"),  QStringLiteral("\\bthen\\b"),
    };

    const QString patterns_blue[] = {
        QStringLiteral("\\bfunction\\b"), QStringLiteral("\\blocal\\b"),
        QStringLiteral("\\breturn\\b"),   QStringLiteral("\\bbreak\\b"),
        QStringLiteral("\\bnil\\b"),      QStringLiteral("\\btrue\\b"),
        QStringLiteral("\\bfalse\\b"),    QStringLiteral("\\bnot\\b"),
        QStringLiteral("\\band\\b"),      QStringLiteral("\\bor\\b"),
    };
    QTextCharFormat keyword_blue_format, keyword_pink_format;
    keyword_blue_format.setForeground(Qt::darkBlue);
    keyword_blue_format.setFontWeight(QFont::Bold);
    keyword_pink_format.setForeground(QBrush(QColor(174, 50, 160)));
    keyword_pink_format.setFontWeight(QFont::Bold);
    for (const QString& pattern : patterns_blue)
    {
        highlighting_rules_.append({QRegularExpression(pattern), keyword_blue_format});
    }
    for (const QString& pattern : patterns_pink)
    {
        highlighting_rules_.append({QRegularExpression(pattern), keyword_pink_format});
    }
}

void ScriptHighlighter::highlightBlock(const QString& text)
{
    if (text.isEmpty())
    {
        return;
    }
    for (const HighlightingRule& rule : qAsConst(highlighting_rules_))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

ScriptEditor::ScriptEditor(bool& open, std::function<void(const std::string&)> run_script_callback,
                           QWidget* parent)
    : QWidget(parent, Qt::Window), open_(open), run_script_callback_(run_script_callback)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Script Editor");
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    editor_ = new QTextEdit(this);
    editor_->setFont(font);
    editor_->setMinimumSize(400, 400);
    highlighter_ = new ScriptHighlighter(editor_->document());
    toolbar_ = new QToolBar(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(toolbar_);
    layout->addWidget(editor_);
    show();
    open_ = true;
}

ScriptEditor::~ScriptEditor()
{
    open_ = false;
}

void ScriptEditor::run_script() {}