#pragma once

#include <functional>
#include <QMessageBox>

template <typename Callable>
static inline void qt_may_throw(Callable func)
{
    try
    {
        func();
    } catch (std::exception& ex)
    {
        QMessageBox messageBox;
        messageBox.addButton(QMessageBox::tr("Sounds like a you problem"), QMessageBox::YesRole);
        messageBox.setWindowTitle("Damn...");
        messageBox.setText(ex.what());
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.exec();
        return;
    }
}