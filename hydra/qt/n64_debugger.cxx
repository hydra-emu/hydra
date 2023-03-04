#include "n64_debugger.hxx"
#include <N64TKP/core/n64_types.hxx>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidgetItem>
#include <QTimer>
#include <QCheckBox>
#include <iostream>
#include <string>
#include <fmt/format.h>

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
                    gpr_edit_[i]->blockSignals(true);
                    gpr_edit_[i]->setText(QString::fromStdString(get_gpr_value(i)));
                    gpr_edit_[i]->blockSignals(false);
                    gpr_edit_[i]->setReadOnly(!emulator_->Paused);
                }
                break;
            }
            case DisassemblerIndex: {
                std::string disasm = emulator_->n64_impl_.cpu_.disassemble(emulator_->n64_impl_.cpu_.pc_, emulator_->n64_impl_.cpu_.pc_ + 0x100, register_names_);
                disassembler_text_->setPlainText(QString::fromStdString(disasm));
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
    disassembler_text_ = new QTextEdit;
    // disassembler_text_->setReadOnly(true);
    disassembler_text_->setFont(fixedfont);
    disassembler_text_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    highlighter_ = new MIPSHighlighter(disassembler_text_->document());
    Disassembler_layout->addWidget(disassembler_text_, 2, 0, 1, 2);
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