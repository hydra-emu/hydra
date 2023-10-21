#pragma once

#include <filesystem>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

class KeyPickerPage : public QWidget
{
    Q_OBJECT

public:
    KeyPickerPage(QWidget* parent = 0);
    void KeyPressed(QKeyEvent* event);

private slots:
    void onComboBoxChange(int index);
    void onCellDoubleClicked(int row, int column);
    void onAddButtonClicked();
    void onRemoveButtonClicked();
    void onSaveButtonClicked();
    void onCancelButtonClicked();
    void onCancelWaitingButtonClicked();

private:
    bool waiting_input_ = false;
    bool adding_mapping_ = false;
    int row_waiting_ = 0;
    QString old_key_;
    QComboBox* emulator_picker_;
    QTabWidget* tab_show_;
    QVBoxLayout* layout_;
    QPushButton* add_button_;
    QPushButton* remove_button_;
    QPushButton* save_button_;
    QPushButton* cancel_button_;
    QPushButton* cancel_waiting_button_;
    std::string get_key(const std::string& emulator_name, const std::string& key);
    void set_key(const std::string& emulator_name, const std::string& key,
                 const std::string& value);
    void ensure_same();
    QTableWidget* make_page(const std::filesystem::path& path);
    QTableWidget* copy_page(QTableWidget* copy = nullptr);
    void add_tab(const QString& name, QTableWidget* table);
};
