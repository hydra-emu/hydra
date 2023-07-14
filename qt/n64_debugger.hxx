#ifndef N64_DEBUGGER
#define N64_DEBUGGER
#include <n64/core/n64_types.hxx>
#include <QWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QTabWidget>
#include <memory>
#include <emulator_types.hxx>
#include <n64/n64_tkpwrapper.hxx>
#include <QFontDatabase>
#include <QSyntaxHighlighter>
#include <QPlainTextEdit>

class QLabel;
class QTextEdit;

#define N64_DEBUGGER_TABS \
    X(Registers) \
    X(Disassembler) \
    X(Settings) \
    X(TMem)

class MIPSHighlighter final : public QSyntaxHighlighter {
    Q_OBJECT
public:
    MIPSHighlighter(QTextDocument *parent = nullptr);
private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    void highlightBlock(const QString& text) override;
    QList<HighlightingRule> highlighting_rules_;
    QTextCharFormat singleline_comment_format_;
    QTextCharFormat instruction_format_;
    QTextCharFormat register_format_;
    QTextCharFormat constant_format_;
    QTextCharFormat punctuator_format_;
    QTextCharFormat label_format_;
};

class N64Disassembler : public QPlainTextEdit {
    Q_OBJECT
public:
    N64Disassembler(bool& register_names, QWidget* parent = nullptr);
    void setInstructions(const std::vector<hydra::N64::DisassemblerInstruction>& instructions);
    void setGPRs(const std::array<hydra::N64::MemDataUnionDW, 32>& gprs);
    void wheelEvent(QWheelEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    bool event(QEvent *e) override;
    void updateText();
    void Goto(uint32_t addr);
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
private:
    std::vector<hydra::N64::DisassemblerInstruction> instructions_;
    std::array<hydra::N64::MemDataUnionDW, 32> gprs_;
    MIPSHighlighter* highlighter_;
    int top_line_ = 0;
    int top_line_pixel_ = 0;
    QWidget* line_number_area_;
    bool& register_names_;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(N64Disassembler* debugger) : QWidget(debugger), debugger_(debugger) {}
    QSize sizeHint() const override
    {
        return QSize(debugger_->lineNumberAreaWidth(), 0);
    }
protected:
    void paintEvent(QPaintEvent* event) override
    {
        debugger_->lineNumberAreaPaintEvent(event);
    }
private:
    N64Disassembler* debugger_;
};

class N64Debugger : public QWidget {
    Q_OBJECT;
public:
    N64Debugger(bool& open, QWidget* parent = nullptr);
    ~N64Debugger();

    void SetEmulator(hydra::N64::N64_TKPWrapper* emulator);
private slots:
    void on_tab_change();
    void update_debugger_tab();
private:
    bool open_;
    bool was_paused_ = false;
    hydra::EmuType emulator_type_;
    hydra::N64::N64_TKPWrapper* emulator_ { nullptr };

    QListWidget* tab_list_;
    QTabWidget* tab_show_;
    QGroupBox* right_group_box_, *left_group_box_;
    QLabel* tmem_image_;
    const QFont fixedfont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    bool register_names_ = false;
    std::array<QLabel*, 32> gpr_edit_names_;
    std::array<QLineEdit*, 32> gpr_edit_;

    bool disassembled_ = false;
    N64Disassembler* disassembler_text_;

    void create_tabs();
    std::string get_gpr_name(int n);
    std::string get_gpr_value(int n);
    void register_changed(const QString&, int reg);

    #define X(name) QWidget* name##_tab; QGridLayout* name##_layout; void create_##name##_tab();
    N64_DEBUGGER_TABS
    #undef X

    enum TabIndex {
        #define X(name) name##Index,
        N64_DEBUGGER_TABS
        #undef X
    };
};
#endif