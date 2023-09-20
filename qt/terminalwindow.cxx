#include "terminalwindow.hxx"
#include <iostream>
#include <log.hxx>
#include <QCheckBox>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <settings.hxx>

std::unordered_map<std::string, std::string> TerminalWindow::logs_;

TerminalWindow::TerminalWindow(bool& open, QWidget* parent)
    : QWidget(parent, Qt::Window), open_(open)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowTitle("Terminal");
    groups_combo_box_ = new QComboBox(this);
    groups_combo_box_->addItem("Warn");
    groups_combo_box_->addItem("Info");
    groups_combo_box_->addItem("Debug");
    connect(groups_combo_box_, &QComboBox::currentTextChanged, this,
            &TerminalWindow::on_group_changed);

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    edit_ = new QTextEdit(this);
    edit_->setReadOnly(true);
    edit_->setLineWrapMode(QTextEdit::NoWrap);
    edit_->setMinimumSize(400, 400);
    edit_->setFont(font);
    QPalette palette = edit_->palette();
    palette.setColor(QPalette::Base, Qt::black);
    palette.setColor(QPalette::Text, Qt::lightGray);
    edit_->setPalette(palette);

    on_group_changed(groups_combo_box_->currentText());

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(groups_combo_box_);
    layout->addWidget(edit_);

    QCheckBox* print_enabled = new QCheckBox("Print to native terminal");
    print_enabled->setChecked(Settings::Get("print_to_native_terminal") == "true");
    connect(print_enabled, &QCheckBox::stateChanged, [](int state) {
        Settings::Set("print_to_native_terminal", state == Qt::Checked ? "true" : "false");
    });
    layout->addWidget(print_enabled);

    setLayout(layout);
    show();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TerminalWindow::on_timeout);
    timer->start(1000);

    open_ = true;
}

TerminalWindow::~TerminalWindow()
{
    open_ = false;
}

void TerminalWindow::on_group_changed(const QString& group)
{
    edit_->setText(QString::fromStdString(logs_[group.toStdString()]));
}

void TerminalWindow::on_timeout()
{
    on_group_changed(groups_combo_box_->currentText());
}

void TerminalWindow::log(const std::string& group, const std::string& message)
{
    logs_[group] += message;
}

void TerminalWindow::Init()
{
    Logger::HookCallback("Warn", [](const std::string& message) { log("Warn", message); });
    Logger::HookCallback("Info", [](const std::string& message) { log("Info", message); });
    Logger::HookCallback("Debug", [](const std::string& message) { log("Debug", message); });

    if (Settings::Get("print_to_native_terminal") == "true")
    {
        Logger::HookCallback(
            "Warn", [](const std::string& message) { std::cout << message << std::flush; });
        // Logger::HookCallback("Info", [](const std::string& message) { std::cout << message <<
        // std::flush; }); Logger::HookCallback("Debug", [](const std::string& message) { std::cout
        // << message << std::flush; });
    }
}