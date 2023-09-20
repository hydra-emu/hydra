#include "shadereditor.hxx"
#include <QMessageBox>
#include <QOpenGLShader>
#include <QVBoxLayout>

#define QT_MAY_THROW(func)                                             \
    try                                                                \
    {                                                                  \
        func                                                           \
    } catch (std::exception & ex)                                      \
    {                                                                  \
        QMessageBox messageBox;                                        \
        messageBox.critical(0, "Shader compilation error", ex.what()); \
        messageBox.setFixedSize(500, 200);                             \
        return;                                                        \
    }

ShaderHighlighter::ShaderHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
{
    const QString patterns_pink[] = {
        QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bcontinue\\b"),
        QStringLiteral("\\bdo\\b"),    QStringLiteral("\\bfor\\b"),
        QStringLiteral("\\bwhile\\b"), QStringLiteral("\\bswitch\\b"),
        QStringLiteral("\\bcase\\b"),  QStringLiteral("\\bdefault\\b"),
        QStringLiteral("\\bif\\b"),    QStringLiteral("\\belse\\b"),
    };
    const QString patterns_blue[] = {
        QStringLiteral("\\battribute\\b"),
        QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\buniform\\b"),
        QStringLiteral("\\bvarying\\b"),
        QStringLiteral("\\blayout\\b"),
        QStringLiteral("\\bcentroid\\b"),
        QStringLiteral("\\bflat\\b"),
        QStringLiteral("\\bsmooth\\b"),
        QStringLiteral("\\bnoperspective\\b"),
        QStringLiteral("\\bpatch\\b"),
        QStringLiteral("\\bsample\\b"),
        QStringLiteral("\\bsubroutine\\b"),
        QStringLiteral("\\bin\\b"),
        QStringLiteral("\\bout\\b"),
        QStringLiteral("\\binout\\b"),
        QStringLiteral("\\bfloat\\b"),
        QStringLiteral("\\bdouble\\b"),
        QStringLiteral("\\bint\\b"),
        QStringLiteral("\\bvoid\\b"),
        QStringLiteral("\\bbool\\b"),
        QStringLiteral("\\btrue\\b"),
        QStringLiteral("\\bfalse\\b"),
        QStringLiteral("\\binvariant\\b"),
        QStringLiteral("\\bdiscard\\b"),
        QStringLiteral("\\breturn\\b"),
        QStringLiteral("\\bmat2\\b"),
        QStringLiteral("\\bmat3\\b"),
        QStringLiteral("\\bmat4\\b"),
        QStringLiteral("\\bdmat2\\b"),
        QStringLiteral("\\bdmat3\\b"),
        QStringLiteral("\\bdmat4\\b"),
        QStringLiteral("\\bmat2x2\\b"),
        QStringLiteral("\\bmat2x3\\b"),
        QStringLiteral("\\bmat2x4\\b"),
        QStringLiteral("\\bdmat2x2\\b"),
        QStringLiteral("\\bdmat2x3\\b"),
        QStringLiteral("\\bdmat2x4\\b"),
        QStringLiteral("\\bmat3x2\\b"),
        QStringLiteral("\\bmat3x3\\b"),
        QStringLiteral("\\bmat3x4\\b"),
        QStringLiteral("\\bdmat3x2\\b"),
        QStringLiteral("\\bdmat3x3\\b"),
        QStringLiteral("\\bdmat3x4\\b"),
        QStringLiteral("\\bmat4x2\\b"),
        QStringLiteral("\\bmat4x3\\b"),
        QStringLiteral("\\bmat4x4\\b"),
        QStringLiteral("\\bdmat4x2\\b"),
        QStringLiteral("\\bdmat4x3\\b"),
        QStringLiteral("\\bdmat4x4\\b"),
        QStringLiteral("\\bvec2\\b"),
        QStringLiteral("\\bvec3\\b"),
        QStringLiteral("\\bvec4\\b"),
        QStringLiteral("\\bivec2\\b"),
        QStringLiteral("\\bivec3\\b"),
        QStringLiteral("\\bivec4\\b"),
        QStringLiteral("\\bbvec2\\b"),
        QStringLiteral("\\bbvec3\\b"),
        QStringLiteral("\\bbvec4\\b"),
        QStringLiteral("\\bdvec2\\b"),
        QStringLiteral("\\bdvec3\\b"),
        QStringLiteral("\\bdvec4\\b"),
        QStringLiteral("\\buint\\b"),
        QStringLiteral("\\buvec2\\b"),
        QStringLiteral("\\buvec3\\b"),
        QStringLiteral("\\buvec4\\b"),
        QStringLiteral("\\blowp\\b"),
        QStringLiteral("\\bmediump\\b"),
        QStringLiteral("\\bhighp\\b"),
        QStringLiteral("\\bprecision\\b"),
        QStringLiteral("\\bsampler1D\\b"),
        QStringLiteral("\\bsampler2D\\b"),
        QStringLiteral("\\bsampler3D\\b"),
        QStringLiteral("\\bsamplerCube\\b"),
        QStringLiteral("\\bsampler1DShadow\\b"),
        QStringLiteral("\\bsampler2DShadow\\b"),
        QStringLiteral("\\bsamplerCubeShadow\\b"),
        QStringLiteral("\\bsampler1DArray\\b"),
        QStringLiteral("\\bsampler2DArray\\b"),
        QStringLiteral("\\bsampler1DArrayShadow\\b"),
        QStringLiteral("\\bsampler2DArrayShadow\\b"),
        QStringLiteral("\\bisampler1D\\b"),
        QStringLiteral("\\bisampler2D\\b"),
        QStringLiteral("\\bisampler3D\\b"),
        QStringLiteral("\\bisamplerCube\\b"),
        QStringLiteral("\\bisampler1DArray\\b"),
        QStringLiteral("\\bisampler2DArray\\b"),
        QStringLiteral("\\busampler1D\\b"),
        QStringLiteral("\\busampler2D\\b"),
        QStringLiteral("\\busampler3D\\b"),
        QStringLiteral("\\busamplerCube\\b"),
        QStringLiteral("\\busampler1DArray\\b"),
        QStringLiteral("\\busampler2DArray\\b"),
        QStringLiteral("\\bsampler2DRect\\b"),
        QStringLiteral("\\bsampler2DRectShadow\\b"),
        QStringLiteral("\\bisampler2DRect\\b"),
        QStringLiteral("\\busampler2DRect\\b"),
        QStringLiteral("\\bsamplerBuffer\\b"),
        QStringLiteral("\\bisamplerBuffer\\b"),
        QStringLiteral("\\busamplerBuffer\\b"),
        QStringLiteral("\\bsampler2DMS\\b"),
        QStringLiteral("\\bisampler2DMS\\b"),
        QStringLiteral("\\busampler2DMS\\b"),
        QStringLiteral("\\bsampler2DMSArray\\b"),
        QStringLiteral("\\bisampler2DMSArray\\b"),
        QStringLiteral("\\busampler2DMSArray\\b"),
        QStringLiteral("\\bsamplerCubeArray\\b"),
        QStringLiteral("\\bsamplerCubeArrayShadow\\b"),
        QStringLiteral("\\bisamplerCubeArray\\b"),
        QStringLiteral("\\busamplerCubeArray\\b"),
        QStringLiteral("\\bstruct\\b"),
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
    multiline_comment_format_.setForeground(Qt::darkGreen);
    singleline_comment_format_.setForeground(Qt::darkGreen);
    hashtag_format_.setForeground(Qt::lightGray);
    highlighting_rules_.append(
        {QRegularExpression(QStringLiteral("//[^\n]*")), singleline_comment_format_});
    highlighting_rules_.append({QRegularExpression(QStringLiteral("#[^\n]*")), hashtag_format_});
    comment_start_expression_ = QRegularExpression(QStringLiteral("/\\*"));
    comment_end_expression_ = QRegularExpression(QStringLiteral("\\*/"));
}

void ShaderHighlighter::highlightBlock(const QString& text)
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
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
    {
        startIndex = text.indexOf(comment_start_expression_);
    }

    while (startIndex >= 0)
    {
        QRegularExpressionMatch match = comment_end_expression_.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiline_comment_format_);
        startIndex = text.indexOf(comment_start_expression_, startIndex + commentLength);
    }
}

ShaderEditor::ShaderEditor(bool& open, std::function<void(QString*, QString*)> callback,
                           QWidget* parent)
    : open_(open), callback_(callback), QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    editor_ = new QTextEdit;
    editor_->setMinimumSize(400, 400);
    editor_->setFont(font);
    highlighter_ = new ShaderHighlighter(editor_->document());
    QFile file(":/shaders/simple.fs");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        editor_->setPlainText(file.readAll());
    }
    toolbar_ = new QToolBar;
    open_act_ = toolbar_->addAction(QIcon(":/images/open.png"), "Open shader");
    connect(open_act_, SIGNAL(triggered()), this, SLOT(open_shader()));
    toolbar_->addSeparator();
    autocompile_act_ = toolbar_->addAction(QIcon(":/images/autocompile.png"), "Auto compile");
    autocompile_act_->setCheckable(true);
    connect(autocompile_act_, SIGNAL(triggered()), this, SLOT(autocompile()));
    compile_act_ = toolbar_->addAction(QIcon(":/images/compile.png"), "Compile");
    connect(compile_act_, SIGNAL(triggered()), this, SLOT(compile()));
    autocompile();
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(toolbar_);
    layout->addWidget(editor_);
    setLayout(layout);
    setWindowTitle(tr("Shader editor"));
    setWindowIcon(QIcon(":/images/shaders.png"));
    show();
    open_ = true;
}

void ShaderEditor::compile()
{
    QT_MAY_THROW(QString src = editor_->toPlainText(); callback_(nullptr, &src););
}

void ShaderEditor::open_shader() {}

void ShaderEditor::autocompile()
{
    autocompile_ ^= true;
    autocompile_act_->setChecked(autocompile_);
    compile_act_->setEnabled(!autocompile_);
}

ShaderEditor::~ShaderEditor()
{
    open_ = false;
}