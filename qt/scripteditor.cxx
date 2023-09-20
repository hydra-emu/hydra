#include "scripteditor.hxx"

#include <error_factory.hxx>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <settings.hxx>

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

ScriptEditor::ScriptEditor(bool& open,
                           std::function<void(const std::string&, bool)> run_script_callback,
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
    auto open_act = new QAction(QIcon(":/images/open.png"), "Open", this);
    connect(open_act, &QAction::triggered, this, &ScriptEditor::open_script);
    open_act->setShortcut(QKeySequence::Open);
    toolbar_->addAction(open_act);
    auto save_act = new QAction(QIcon(":/images/save.png"), "Save", this);
    connect(save_act, &QAction::triggered, this, &ScriptEditor::save_script);
    save_act->setShortcut(QKeySequence::Save);
    toolbar_->addAction(save_act);
    toolbar_->addSeparator();
    toolbar_->addAction(QIcon(":/images/run.png"), "Run", this, &ScriptEditor::run_script);
    QCheckBox* safe_mode = new QCheckBox("Enable safe mode");
    connect(safe_mode, &QCheckBox::stateChanged, this, &ScriptEditor::safe_mode_changed);
    bool safe_mode_disabled = Settings::Get("lua_safe_mode") == "false";
    safe_mode_ = !safe_mode_disabled;
    safe_mode->setCheckState(safe_mode_disabled ? Qt::Unchecked : Qt::Checked);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(toolbar_);
    layout->addWidget(editor_);
    layout->addWidget(safe_mode);
    show();
    open_ = true;
}

ScriptEditor::~ScriptEditor()
{
    open_ = false;
}

void ScriptEditor::run_script()
{
    run_script_callback_(editor_->toPlainText().toStdString(), safe_mode_);
}

void ScriptEditor::open_script()
{
    QFileDialog dialog(this);
    dialog.setDirectory(Settings::Get("last_script_path").c_str());
    QString filename = dialog.getOpenFileName(
        this, "Open Script", Settings::Get("last_script_path").c_str(), "Lua Script (*.lua)");
    if (filename.isEmpty())
    {
        return;
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to open file");
    }
    Settings::Set("last_script_path", std::filesystem::path(filename.toStdString()).parent_path());
    editor_->setPlainText(file.readAll());
}

void ScriptEditor::save_script()
{
    QFileDialog dialog(this);
    dialog.setDefaultSuffix("lua");
    QString filename = dialog.getSaveFileName(
        this, "Save Script", Settings::Get("last_script_path").c_str(), "Lua Script (*.lua)");
    if (filename.isEmpty())
    {
        return;
    }
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to open file");
    }
    Settings::Set("last_script_path", std::filesystem::path(filename.toStdString()).parent_path());
    file.write(editor_->toPlainText().toUtf8());
}

void ScriptEditor::safe_mode_changed(int state)
{
    if (state == Qt::Unchecked)
    {
        if (Settings::Get("lua_warn_safe_mode") != "disabled")
        {
            QMessageBox warning;
            warning.setWindowTitle("Warning");
            warning.setIcon(QMessageBox::Warning);
            warning.setText("Disabling safe mode can be dangerous, make sure to run scripts from "
                            "trusted sources. Are you sure you want to do this?");
            warning.addButton(QMessageBox::Yes);
            warning.addButton(QMessageBox::No);
            QPushButton* disable_warning = new QPushButton("Don't show again");
            warning.addButton(disable_warning, QMessageBox::ActionRole);
            warning.setDefaultButton(QMessageBox::No);
            warning.exec();

            if (warning.clickedButton() == disable_warning)
            {
                Settings::Set("lua_warn_safe_mode", "disabled");
            }
            else if (warning.clickedButton() == warning.button(QMessageBox::No))
            {
                return;
            }
        }
    }
    safe_mode_ = state == Qt::Checked;
    Settings::Set("lua_safe_mode", safe_mode_ ? "true" : "false");
}