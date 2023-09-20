#include "terminalwindow.hxx"
#include <iostream>
#include <log.hxx>
#include <QCheckBox>
#include <QFileDialog>
#include <QIODevice>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <settings.hxx>

std::unordered_map<std::string, std::string> TerminalWindow::logs_;
bool TerminalWindow::changed_ = false;

TerminalWindow::TerminalWindow(bool& open, QWidget* parent)
    : QWidget(parent, Qt::Window), open_(open)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowTitle("Terminal");
    QToolBar* toolbar = new QToolBar;
    groups_combo_box_ = new QComboBox;
    groups_combo_box_->addItem("Warn");
    groups_combo_box_->addItem("Info");
    groups_combo_box_->addItem("Debug");
    connect(groups_combo_box_, &QComboBox::currentTextChanged, this,
            &TerminalWindow::on_group_changed);
    toolbar->addWidget(groups_combo_box_);
    toolbar->addSeparator();
    QAction* clear_action = toolbar->addAction("Clear");
    clear_action->setIcon(QIcon(":/images/trash.png"));
    connect(clear_action, &QAction::triggered, [this]() {
        logs_[groups_combo_box_->currentText().toStdString()] = "";
        on_group_changed(groups_combo_box_->currentText());
    });
    QAction* save_action = toolbar->addAction("Save");
    save_action->setIcon(QIcon(":/images/save.png"));
    connect(save_action, &QAction::triggered, [this]() {
        QFileDialog dialog(this);
        dialog.setDefaultSuffix("txt");
        QString filename = dialog.getSaveFileName(
            this, "Save log file", Settings::Get("log_directory").c_str(), "Text files (*.txt)");
        if (filename.isEmpty())
            return;
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
        file.write(edit_->toPlainText().toUtf8());
        file.close();

        std::filesystem::path path = filename.toStdString();
        Settings::Set("log_directory", path.parent_path().string());
    });
    save_action->setShortcut(QKeySequence::Save);

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
    layout->addWidget(toolbar);
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
    if (changed_)
        on_group_changed(groups_combo_box_->currentText());
    changed_ = false;
}

void TerminalWindow::log(const std::string& group, const std::string& message)
{
    logs_[group] += message;
    changed_ = true;
}

void TerminalWindow::Init()
{
    Logger::HookCallback("Warn", [](const std::string& message) { log("Warn", message); });
    Logger::HookCallback("Info", [](const std::string& message) { log("Info", message); });
    Logger::HookCallback("Debug", [](const std::string& message) { log("Debug", message); });

    if (Settings::Get("print_to_native_terminal") == "true")
    {
        Logger::HookCallback("Warn", [](const std::string& message) {
            std::cout << "[Warn] " << message << std::flush;
        });
        Logger::HookCallback("Info", [](const std::string& message) {
            std::cout << "[Info] " << message << std::flush;
        });
    }
}