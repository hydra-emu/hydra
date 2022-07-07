#pragma once
#ifndef TKP_DEBUGGER_H
#define TKP_DEBUGGER_H
#include <QWidget>
#include <QListWidget>

class DebuggerWindow : public QWidget {
    Q_OBJECT
private:
    bool& open_;
public:
    DebuggerWindow(bool& open);
    ~DebuggerWindow();

};
#endif