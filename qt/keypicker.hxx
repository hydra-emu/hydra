#pragma once

#include <QComboBox>
#include <QLabel>
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

private:
    bool waiting_input_ = false;
    int row_waiting_ = 0;
    QComboBox* emulator_picker_;
    QTabWidget* tab_show_;
    QVBoxLayout* layout_;
    void saveKeySettings();
    std::string get_key(const std::string& emulator_name, const std::string& key);
    void set_key(const std::string& emulator_name, const std::string& key,
                 const std::string& value);
};
