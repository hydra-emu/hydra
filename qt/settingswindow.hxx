#pragma once

#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QWidget>

class InputPage;
class QGridLayout;
class QComboBox;

class SettingsWindow : public QWidget
{
    Q_OBJECT

public:
    SettingsWindow(std::function<void(int)> volume_callback, QWidget* parent = nullptr);
    ~SettingsWindow() = default;

private slots:
    void on_tab_change(int tab);

private:
    QListWidget* tab_list_;
    QTabWidget* tab_show_;
    QGroupBox *right_group_box_, *left_group_box_;
    InputPage* key_picker_;
    std::function<void(int)> volume_callback_;
    std::vector<std::tuple<QComboBox*, int, QString>> listener_combos_;
    void keyPressEvent(QKeyEvent* event);
    void create_tabs();
    void on_open_file_click(QLineEdit* edit, const std::string& name, const std::string& setting,
                            const std::string& extension,
                            std::function<void(const std::string&)> callback);
    void on_open_dir_click(QLineEdit* edit, const std::string& name, const std::string& setting,
                           std::function<void(const std::string&)> callback);
    void add_filepicker(QGridLayout* layout, const std::string& name, const std::string& setting,
                        const std::string& extension, int row, int column, bool dir = false,
                        std::function<void(const std::string&)> callback = {});
    QComboBox* make_input_combo(const QString& core_name, int player,
                                const QString& selected_mapping);
};
