#include "traceloggerwindow.hxx"
#include <QVBoxLayout>
#include <QGroupBox>
#include <include/emulator_factory.h>

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
        QHBoxLayout* bot_layout = new QHBoxLayout;

        bot_qgb->setLayout(bot_layout);
    }
    layout->addWidget(top_qgb);
    layout->addWidget(bot_qgb);
    setLayout(layout);
    setWindowTitle("Tracelogger");
    show();
    open_ = true;
}

TraceloggerWindow::~TraceloggerWindow() {
    open_ = false;
}

void TraceloggerWindow::Reset(std::shared_ptr<TKPEmu::Tools::MQBase> mq, TKPEmu::EmuType type) {
    
}