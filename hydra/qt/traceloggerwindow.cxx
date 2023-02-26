#include "traceloggerwindow.hxx"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QScrollBar>
#include <utility>
#include <include/emulator_factory.h>
#define emu_data TKPEmu::EmulatorFactory::GetEmulatorUserData()[static_cast<int>(emulator_type_)]

TraceloggerWindow::TraceloggerWindow(bool& open, std::shared_ptr<TKPEmu::Tools::MQBase> mq, TKPEmu::EmuType type, QWidget* parent) :
    open_(open),
    message_queue_(mq),
    emulator_type_(type),
    QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout* layout = new QVBoxLayout;
    QGroupBox* top_qgb = new QGroupBox;
    QGroupBox* bot_qgb = new QGroupBox;
    {
        QVBoxLayout* top_layout = new QVBoxLayout;
        QListWidget* tab_list = new QListWidget;
        const auto& data = TKPEmu::EmulatorFactory::GetEmulatorData().at(static_cast<int>(emulator_type_));
        for (const auto& str : data.LoggingOptions) {
            QListWidgetItem* item = new QListWidgetItem(str.c_str());
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            checkboxes_.push_back(item);
            tab_list->addItem(item);
        }
        top_layout->addWidget(tab_list);
        top_qgb->setLayout(top_layout);
    }
    {
        QGridLayout* bot_layout = new QGridLayout;
        text_edit_ = new QPlainTextEdit;
        QFontMetrics m(text_edit_->font());
        text_edit_->setReadOnly(true);
        text_edit_->verticalScrollBar()->hide();
        text_edit_->setFixedHeight(m.lineSpacing() * 1.5);
        log_path_ = emu_data.Get("log_path").c_str();
        text_edit_->setPlaceholderText("Log directory");
        if (log_path_ != "" && !log_path_.isEmpty())
            text_edit_->setPlainText(log_path_);
        QPushButton* browse_button = new QPushButton;
        browse_button->setText("...");
        connect(browse_button, SIGNAL(clicked()), this, SLOT(browse_clicked()));
        log_button_ = new QPushButton;
        log_button_->setText("Start logging");
        connect(log_button_, SIGNAL(clicked()), this, SLOT(log_clicked()));
        bot_layout->addWidget(text_edit_, 0, 0);
        bot_layout->addWidget(browse_button, 0, 1);
        bot_layout->addWidget(log_button_, 1, 0);
        bot_qgb->setLayout(bot_layout);
        bot_qgb->setFixedHeight(100);
        bot_qgb->setMinimumWidth(400);
    }
    layout->addWidget(top_qgb);
    layout->addWidget(bot_qgb);
    setLayout(layout);
    setWindowTitle("Tracelogger");
    show();
    open_ = true;
}

TraceloggerWindow::~TraceloggerWindow() {
}

void TraceloggerWindow::browse_clicked() {
    log_path_ = QFileDialog::getExistingDirectory(this, tr("Select log directory..."), QString(), QFileDialog::ShowDirsOnly);
    text_edit_->setPlainText(log_path_);
    emu_data.Set("log_path", log_path_.toStdString());
}

void TraceloggerWindow::log_clicked() {
    if (!is_logging_) {
        is_logging_ = true;
        auto path = log_path_.toStdString() + "/log";
        int i = 1;
        while (true) {
            if (!std::filesystem::exists(path + std::to_string(i) + ".txt"))
                break;
            i++;
        }
        std::pair<std::string, std::bitset<64>> data {};
        data.first = path + std::to_string(i) + ".txt";
        for (int i = 0; i < checkboxes_.size(); i++) {
            data.second.set(i, checkboxes_[i]->checkState() == Qt::Checked);
        }
        Request req = {
            .Id = RequestId::COMMON_START_LOG,
            .Data = std::move(data)
        };
        message_queue_->PushRequest(std::move(req));
        log_button_->setText("Stop logging");
    } else {
        is_logging_ = false;
        Request req = {
            .Id = RequestId::COMMON_STOP_LOG
        };
        message_queue_->PushRequest(std::move(req));
        log_button_->setText("Start logging");
    }
}

void TraceloggerWindow::Reset(std::shared_ptr<TKPEmu::Tools::MQBase> mq, TKPEmu::EmuType type) {
    
}