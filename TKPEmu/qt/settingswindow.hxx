#pragma once
#ifndef TKP_SETTINGS_H
#define TKP_SETTINGS_H
#include <QWidget>
#include <QListWidget>

class SettingsWindow : public QWidget {
    Q_OBJECT
private:
    bool& open_;
public:
    SettingsWindow(bool& open, QWidget* parent = nullptr);
    ~SettingsWindow();
};
#endif