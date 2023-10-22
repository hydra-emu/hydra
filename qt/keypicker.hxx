#pragma once

#include <filesystem>
#include <input.hxx>
#include <QComboBox>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

class InputPage : public QWidget
{
    Q_OBJECT

public:
    InputPage(QWidget* parent = 0);
    void KeyPressed(QKeyEvent* event);

private slots:
    void set_tab(int index);
    void onCellDoubleClicked(int row, int column);
    void onAddButtonClicked();
    void onCopyButtonClicked();
    void onRemoveButtonClicked();
    void onSaveButtonClicked();
    void onCancelButtonClicked();
    void onCancelWaitingButtonClicked();

private:
    bool waiting_input_ = false;
    bool adding_mapping_ = false;
    bool is_copying_page_ = false;
    int row_waiting_ = 0;
    QString old_key_;
    QComboBox* emulator_picker_;
    QTabWidget* tab_show_;
    QVBoxLayout* layout_;
    QPushButton* add_button_;
    QPushButton* copy_button_;
    QPushButton* remove_button_;
    QPushButton* save_button_;
    QPushButton* cancel_button_;
    QPushButton* cancel_waiting_button_;
    QLineEdit* mapping_name_;
    std::string get_key(const std::string& emulator_name, const std::string& key);
    void set_key(const std::string& emulator_name, const std::string& key,
                 const std::string& value);
    void ensure_same();
    QTableWidget* make_page(const std::filesystem::path& path);
    QTableWidget* copy_page(QTableWidget* copy = nullptr);
    void save_page(QTableWidget* table);
    void add_tab_from_file(QFile& file);
    void add_tab(const QString& name, QTableWidget* table);
    void cancel_waiting();

    enum ButtonPage
    {
        Normal,
        AddingMapping,
        WaitingInput
    };

    void set_buttons(ButtonPage page);
    bool check_exists(const QString& name);
    hydra::KeyMappings table_to_mappings(QTableWidget* table);
};
