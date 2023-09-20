#pragma once

#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QWidget>

class QGridLayout;

class SettingsWindow : public QWidget
{
    Q_OBJECT

private:
    bool& open_;
    QListWidget* tab_list_;
    QTabWidget* tab_show_;
    QGroupBox *right_group_box_, *left_group_box_;
    QLineEdit *dmg_bios_path_, *cgb_bios_path_;
    QLineEdit* ipl_path_;
    std::function<void(int)> volume_callback_;
    void create_tabs();
    void on_open_file_click(const std::string& setting, const std::string& extension);
    void add_filepicker(QGridLayout* layout, const std::string& name, const std::string& setting,
                        const std::string& extension, int row, int column);
private slots:
    void on_tab_change();

public:
    SettingsWindow(bool& open, std::function<void(int)> volume_callback, QWidget* parent = nullptr);
    ~SettingsWindow();
};
