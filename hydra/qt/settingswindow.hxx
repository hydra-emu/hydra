#pragma once
#ifndef TKP_SETTINGS_H
#define TKP_SETTINGS_H
#include <QWidget>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>

class SettingsWindow : public QWidget {
    Q_OBJECT
private:
    bool& open_;
    QListWidget* tab_list_;
    QTabWidget* tab_show_;
    QGroupBox* right_group_box_, *left_group_box_;
    QLineEdit* dmg_bios_path_, *cgb_bios_path_;
    QLineEdit* ipl_path_;
    void create_tabs();
private slots:
    void on_tab_change();
    void on_dmg_click();
    void on_cgb_click();
    void on_ipl_click();
    void on_gb_skip_bios_click(int state);
public:
    SettingsWindow(bool& open, QWidget* parent = nullptr);
    ~SettingsWindow();
};
#endif