#include "debuggerwindow.hxx"
#include <QGridLayout>

DebuggerWindow::DebuggerWindow(bool& open, std::shared_ptr<TKPEmu::Tools::MQBase> mq, QWidget* parent) :
    open_(open),
    message_queue_(mq),
    QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose);
    combo_box_ = new QComboBox;
    QGridLayout* layout = new QGridLayout;
    layout->addWidget(combo_box_);
    setLayout(layout);
    setWindowTitle("Debugger");
    show();
    open_ = true;
}

DebuggerWindow::~DebuggerWindow() {
    open_ = false;
}