#include "traceloggerwindow.hxx"
#include <QGridLayout>
#include <QGroupBox>
#include <QListWidget>
#include <include/emulator_factory.h>

TraceloggerWindow::TraceloggerWindow(bool& open, std::shared_ptr<TKPEmu::Tools::MQBase> mq, TKPEmu::EmuType type, QWidget* parent) :
    open_(open),
    message_queue_(mq),
    emulator_type_(type),
    QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose);
    {
        QListWidget* tab_list = new QListWidget;
        const auto& data = TKPEmu::EmulatorFactory::GetEmulatorData().at(static_cast<int>(emulator_type_));
        for (const auto& str : data.LoggingOptions) {
            QListWidgetItem* item = new QListWidgetItem(str.c_str());
            item->setFlags(Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            tab_list->addItem(item);
        }
    }
}

TraceloggerWindow::~TraceloggerWindow() {
    open_ = false;
}