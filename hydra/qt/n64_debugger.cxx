#include "n64_debugger.hxx"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidgetItem>
#include <QTimer>
#include <QCheckBox>
#include <QPainter>
#include <QToolTip>
#include <QMessageBox>
#include <iostream>
#include <string>
#include <fmt/format.h>
#include <include/log.hxx>

MIPSHighlighter::MIPSHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    instruction_format_.setForeground(QBrush(QColor(154, 134, 214)));
    register_format_.setForeground(QBrush(QColor(6, 170, 112)));
    constant_format_.setForeground(QBrush(QColor(221, 170, 13)));
    singleline_comment_format_.setForeground(QBrush(QColor(85, 170, 0)));
    label_format_.setForeground(QBrush(QColor(170, 170, 127)));
    punctuator_format_.setForeground(QBrush(QColor(170, 0, 0)));
    for (int i = 0; i < TKPEmu::N64::OperationCodes.size(); i++) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(QString::fromStdString(std::string("\\b") + TKPEmu::N64::OperationCodes[i] + std::string("\\b")));
        rule.format = instruction_format_;
        highlighting_rules_.append(rule);
        rule.pattern = QRegularExpression(QString::fromStdString(std::string("\\b") + TKPEmu::N64::SpecialCodes[i] + std::string("\\b")));
        highlighting_rules_.append(rule);
    }
    for (int i = 0; i < 32; i++) {
        HighlightingRule rule;
        rule.format = register_format_;
        rule.pattern = QRegularExpression(QString::fromStdString(std::string("\\b") + TKPEmu::N64::gpr_get_name(i, false) + std::string("\\b")));
        highlighting_rules_.append(rule);
        rule.pattern = QRegularExpression(QString::fromStdString(std::string("\\b") + TKPEmu::N64::gpr_get_name(i, true) + std::string("\\b")));
        highlighting_rules_.append(rule);
    }
    {
        HighlightingRule rule;
        rule.format = constant_format_;
        rule.pattern = QRegularExpression(QString::fromStdString(std::string("\\b0x[0-9a-fA-F]+\\b")));
        highlighting_rules_.append(rule);
    }
    {
        HighlightingRule rule;
        rule.format = label_format_;
        rule.pattern = QRegularExpression(QString::fromStdString(std::string("lbl_[0-9a-zA-Z]+")));
        highlighting_rules_.append(rule);
    }
    {
        HighlightingRule rule;
        rule.format = punctuator_format_;
        rule.pattern = QRegularExpression(QString::fromStdString(std::string("[\\(\\),:]")));
        highlighting_rules_.append(rule);
    }
    {
        HighlightingRule rule;
        rule.format = singleline_comment_format_;
        rule.pattern = QRegularExpression(QString::fromStdString(std::string(";.*")));
        highlighting_rules_.append(rule);
    }
}

void MIPSHighlighter::highlightBlock(const QString& text) {
    if (text.isEmpty())
        return;
    for (const HighlightingRule &rule : qAsConst(highlighting_rules_)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

N64Disassembler::N64Disassembler(bool& register_names, QWidget* parent) : register_names_(register_names), QPlainTextEdit(parent)
{
    highlighter_ = new MIPSHighlighter(document());
    setReadOnly(true);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setWordWrapMode(QTextOption::NoWrap);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(200);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    line_number_area_ = new LineNumberArea(this);
    connect(this, &N64Disassembler::blockCountChanged, this, &N64Disassembler::updateLineNumberAreaWidth);
    connect(this, &N64Disassembler::updateRequest, this, &N64Disassembler::updateLineNumberArea);
    updateLineNumberAreaWidth(0);
    setMouseTracking(true);
}

bool N64Disassembler::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QPoint pos = helpEvent->pos() - QPoint(lineNumberAreaWidth(), 0);
        QTextCursor cursor = cursorForPosition(pos);
        cursor.select(QTextCursor::WordUnderCursor);
        if (!cursor.selectedText().isEmpty()) {
            std::string reg_value;
            bool is_reg = false;
            if (cursor.selectedText().length() <= 4) {
                auto reg = cursor.selectedText();
                if (!register_names_) {
                    if (reg[0] == 'r') {
                        QString int_s = reg.mid(1);
                        bool ok = false;
                        int i = int_s.toInt(&ok);
                        if (ok) {
                            reg_value = fmt::format("0x{:016x}", gprs_[i].UD);
                            is_reg = true;
                        }
                    }
                } else {
                    for (int i = 0; i < 32; i++) {
                        // Slow, but I'm not writing a case for each alternative register name
                        if (reg == QString::fromStdString(TKPEmu::N64::gpr_get_name(i, true))) {
                            reg_value = fmt::format("0x{:016x}", gprs_[i].UD);
                            is_reg = true;
                            break;
                        }
                    }
                }
            }
            if (is_reg) {
                QToolTip::showText(helpEvent->globalPos(), QString::fromStdString(reg_value));
            } else {
                QToolTip::hideText();
            }
        } else {
            QToolTip::hideText();
        }
        return true;
    }
    return QPlainTextEdit::event(event);
}

int N64Disassembler::lineNumberAreaWidth() {
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * 10;
    return space;
}

void N64Disassembler::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void N64Disassembler::setInstructions(const std::vector<TKPEmu::N64::DisassemblerInstruction>& instructions) {
    instructions_ = instructions;
}

void N64Disassembler::setGPRs(const std::array<TKPEmu::N64::MemDataUnionDW, 32>& gprs) {
    gprs_ = gprs;
}

void N64Disassembler::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        line_number_area_->scroll(0, dy);
    else
        line_number_area_->update(0, rect.y(), line_number_area_->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void N64Disassembler::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);
    updateText();
    QRect cr = contentsRect();
    line_number_area_->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void N64Disassembler::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    if (instructions_.empty())
        return;
    QPainter painter(line_number_area_);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = "0x" + QString::number(instructions_[top_line_ + blockNumber].vaddr, 16);
            painter.setPen(Qt::black);
            painter.drawText(0, top, line_number_area_->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void N64Disassembler::wheelEvent(QWheelEvent *e) {
    top_line_pixel_ -= e->angleDelta().y();
    if (top_line_pixel_ < 0)
        top_line_pixel_ = 0;
    top_line_ = top_line_pixel_ / fontMetrics().height();
    updateText();
}

void N64Disassembler::updateText() {
    int doc_size = size().height();
    int font_height = fontMetrics().height();
    if (font_height == 0)
        return;
    int lines = top_line_ + doc_size / font_height;
    if (lines > instructions_.size())
        lines = instructions_.size();
    std::string text;
    if (lines != 0) {
        for (int i = top_line_; i < lines - 1; i++) {
            text += instructions_.at(i).disassembly + "\n";
        }
        text += instructions_.at(lines - 1).disassembly;
    }
    setPlainText(QString::fromStdString(text));
}

void N64Disassembler::Goto(uint32_t addr) {
    for (int i = 0; i < instructions_.size(); i++) {
        if (instructions_.at(i).vaddr == addr) {
            top_line_ = i;
            top_line_pixel_ = top_line_ * fontMetrics().height();
            updateText();
            return;
        }
    }
    QMessageBox::warning(this, "Error", "Address not found");
}

std::string N64Debugger::get_gpr_value(int n) {
    std::shared_lock lock(emulator_->DataMutex);
    return fmt::format("0x{:016x}", emulator_->n64_impl_.cpu_.gpr_regs_[n].UD);
}

std::string N64Debugger::get_gpr_name(int n) {
    return TKPEmu::N64::gpr_get_name(n, register_names_);
}

N64Debugger::N64Debugger(bool& open, QWidget* parent)
    : open_(open),
    emulator_type_(TKPEmu::EmuType::N64),
    QWidget(parent, Qt::Window)
{
    QGridLayout* main_layout = new QGridLayout;
    left_group_box_ = new QGroupBox;
    right_group_box_ = new QGroupBox;
    {
        tab_list_ = new QListWidget;
        QVBoxLayout* left_layout = new QVBoxLayout;
        left_layout->addWidget(tab_list_);
        left_group_box_->setLayout(left_layout);
        left_group_box_->setFixedWidth(200);
        left_group_box_->setMinimumHeight(400);
        connect(tab_list_, SIGNAL(itemSelectionChanged()), this, SLOT(on_tab_change()));
    }
    {
        tab_show_ = new QTabWidget;
        tab_show_->tabBar()->hide();
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(5, 5, 5, 5);
        layout->addWidget(tab_show_);
        right_group_box_->setLayout(layout);
        right_group_box_->setMinimumWidth(500);
        right_group_box_->setMinimumHeight(400);
    }
    create_tabs();
    main_layout->addWidget(left_group_box_, 0, 0, 1, 1);
    main_layout->addWidget(right_group_box_, 0, 1, 1, 5);
    main_layout->setColumnStretch(0, 0);
    main_layout->setColumnStretch(1, 1);
    setLayout(main_layout);
    setWindowTitle("Debugger");
    show();
    open_ = true;

    QTimer *timer = new QTimer(this);
    timer->start(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_debugger_tab()));
}

N64Debugger::~N64Debugger() {}

void N64Debugger::pi_dma() {
    emulator_->n64_impl_.cpu_.queue_event(SchedulerEventType::Pi, 100);
}

void N64Debugger::si_dma() {
    emulator_->n64_impl_.cpu_.queue_event(SchedulerEventType::Si, 100);
}

void N64Debugger::vi_dma() {
    emulator_->n64_impl_.cpu_.queue_event(SchedulerEventType::Vi, 100);
}

void N64Debugger::on_tab_change() {
    tab_show_->setCurrentIndex(tab_list_->currentRow());
}

void N64Debugger::SetEmulator(TKPEmu::N64::N64_TKPWrapper* emulator) {
    emulator_ = emulator;
}

void N64Debugger::update_debugger_tab() {
    std::shared_lock lock(emulator_->DataMutex);
    if (!emulator_->Paused || !was_paused_) {
        was_paused_ = emulator_->Paused;
        switch (tab_list_->currentRow()) {
            case RegistersIndex: {
                for (int i = 0; i < 32; i++) {
                    if (QString::fromStdString(get_gpr_value(i)) != gpr_edit_[i]->text()) {
                        gpr_edit_[i]->blockSignals(true);
                        gpr_edit_[i]->setText(QString::fromStdString(get_gpr_value(i)));
                        gpr_edit_[i]->blockSignals(false);
                        gpr_edit_[i]->setReadOnly(!was_paused_);
                    }
                }
                break;
            }
            case DisassemblerIndex: {
                if (!disassembled_) {
                    auto disasm = emulator_->n64_impl_.cpu_.disassemble(0x8000'0000, 0x8080'0000, register_names_);
                    disassembler_text_->setInstructions(disasm);
                    disassembler_text_->setGPRs(emulator_->n64_impl_.cpu_.gpr_regs_);
                    disassembler_text_->updateText();
                    disassembled_ = true;
                }
                break;
            }
        }
    }
}

void N64Debugger::create_tabs() {
    #define X(name) QListWidgetItem* name = new QListWidgetItem(#name); \
                    tab_list_->addItem(name); name##_tab = new QWidget; \
                    tab_show_->addTab(name##_tab, #name); \
                    name##_layout = new QGridLayout; \
                    name##_tab->setLayout(name##_layout); \
                    name##_layout->addWidget(new QLabel(#name), 0, 0, 1, 1); \
                    QFrame* name##_line = new QFrame; \
                    name##_line->setFrameShape(QFrame::HLine); \
                    name##_line->setFrameShadow(QFrame::Sunken); \
                    name##_layout->addWidget(name##_line, 1, 0, 1, 2); \
                    create_##name##_tab();
    N64_DEBUGGER_TABS
    #undef X
    tab_list_->setCurrentItem(Registers);
    tab_show_->setCurrentIndex(0);
}

void N64Debugger::register_changed(const QString&, int reg) {
    std::unique_lock lock(emulator_->DataMutex);
    bool ok = false;
    auto old = emulator_->n64_impl_.cpu_.gpr_regs_[reg].UD;
    emulator_->n64_impl_.cpu_.gpr_regs_[reg].UD = gpr_edit_[reg]->text().toULongLong(&ok, 16);
    if (!ok) {
        emulator_->n64_impl_.cpu_.gpr_regs_[reg].UD = old;
    }
}

void N64Debugger::create_Registers_tab() {
    for (int i = 0; i < 32; i++) {
        QLabel*& label = gpr_edit_names_[i];
        label = new QLabel(QString::fromStdString(get_gpr_name(i)));
        Registers_layout->addWidget(label, i + 2, 0, 1, 1);
        QLineEdit*& text = gpr_edit_[i];
        text = new QLineEdit;
        text->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
        text->setInputMask("\\0\\xHHHHHHHHHHHHHHHH");
        text->setText("0x0000000000000000");
        text->setReadOnly(true);
        text->setFont(fixedfont);
        connect(text, &QLineEdit::textChanged, this, std::bind(&N64Debugger::register_changed, this, std::placeholders::_1, i));
        Registers_layout->addWidget(text, i + 2, 1, 1, 1);
    }
    Registers_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 100, 0, 1, 1);
}

void N64Debugger::create_Disassembler_tab() {
    disassembler_text_ = new N64Disassembler(register_names_);
    Disassembler_layout->addWidget(disassembler_text_, 2, 0, 1, 2);
    QPushButton* goto_button = new QPushButton("Goto");
    QLineEdit* goto_edit = new QLineEdit;
    goto_edit->setInputMask("\\0\\xHHHHHHHH");
    connect(goto_button, &QPushButton::clicked, this, [this, goto_edit]() {
        bool ok = false;
        auto addr = goto_edit->text().toULongLong(&ok, 16);
        if (ok) {
            disassembler_text_->Goto(addr);
        }
    });
    Disassembler_layout->addWidget(goto_edit, 0, 1, 1, 1);
    Disassembler_layout->addWidget(goto_button, 0, 2, 1, 1);
    QPushButton* goto_pc_button = new QPushButton("Goto PC");
    connect(goto_pc_button, &QPushButton::clicked, this, [this]() {
        Logger::Info(fmt::format("Goto PC: {:#x}", emulator_->n64_impl_.cpu_.pc_));
        disassembler_text_->Goto(emulator_->n64_impl_.cpu_.pc_);
    });
    Disassembler_layout->addWidget(goto_pc_button, 0, 3, 1, 1);
    Disassembler_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 100, 0, 1, 1);
}

void N64Debugger::create_Settings_tab() {
    QCheckBox* register_name_type = new QCheckBox("Use register name instead of number");
    connect(register_name_type, &QCheckBox::stateChanged, this, [this](int state) {
        register_names_ = state == Qt::Checked;
        for (int i = 0; i < 32; i++) {
            gpr_edit_names_[i]->setText(QString::fromStdString(get_gpr_name(i)));
        }
    });
    Settings_layout->addWidget(register_name_type, 2, 0, 1, 1);
    Settings_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 100, 0, 1, 1);
}

void N64Debugger::create_Interrupts_tab() {
    QPushButton* interrupt_button = new QPushButton("Interrupt");
    connect(interrupt_button, &QPushButton::clicked, this, [this]() {
        emulator_->n64_impl_.cpu_.queue_event(SchedulerEventType::Vi, 0);
    });
    Interrupts_layout->addWidget(interrupt_button, 2, 0, 1, 1);
}