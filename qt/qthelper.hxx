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
        messageBox.critical(0, "Error", ex.what());
        messageBox.setFixedSize(500, 200);
        return;
    }
}