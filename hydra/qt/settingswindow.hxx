#pragma once
#ifndef TKP_SETTINGS_H
#define TKP_SETTINGS_H
#include <QWidget>
#include <QGroupBox>
#include <QListWidget>

class SettingsWindow : public QWidget {
    Q_OBJECT
private:
    bool& open_;
    QListWidget* tab_list_;
    QTabWidget* tab_show_;
    QGroupBox* right_group_box_, *left_group_box_;
    void create_tabs();
private slots:
    void on_tab_change();
public:
    SettingsWindow(bool& open, QWidget* parent = nullptr);
    ~SettingsWindow();
};
#endif