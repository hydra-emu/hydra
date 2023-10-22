#pragma once

#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QWidget>

class InputPage;
class QGridLayout;

class SettingsWindow : public QWidget
{
    Q_OBJECT

private:
    bool& open_;
    QListWidget* tab_list_;
    QTabWidget* tab_show_;
    QGroupBox *right_group_box_, *left_group_box_;
    InputPage* key_picker_;
    std::function<void(int)> volume_callback_;
    void create_tabs();
    void on_open_file_click(QLineEdit* edit, const std::string& name, const std::string& setting,
                            const std::string& extension,
                            std::function<void(const std::string&)> callback);
    void on_open_dir_click(QLineEdit* edit, const std::string& name, const std::string& setting,
                           std::function<void(const std::string&)> callback);
    void add_filepicker(QGridLayout* layout, const std::string& name, const std::string& setting,
                        const std::string& extension, int row, int column, bool dir = false,
                        std::function<void(const std::string&)> callback = {});
    void keyPressEvent(QKeyEvent* event);

private slots:
    void on_tab_change(int tab);

public:
    SettingsWindow(bool& open, std::function<void(int)> volume_callback, QWidget* parent = nullptr);
    ~SettingsWindow();
};
