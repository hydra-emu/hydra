#pragma once

#include <emulator_types.hxx>
#include <QComboBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

class KeyPickerPage : public QWidget
{
    Q_OBJECT

  public:
    KeyPickerPage(QWidget* parent = 0);
  private slots:
    void onComboBoxChange(int index);
    void onCellDoubleClicked(int row, int column);

  private:
    void keyPressEvent(QKeyEvent* event) override;
    bool waiting_input_ = false;
    int row_waiting_ = 0;
    QComboBox* emulator_picker_;
    QTabWidget* tab_show_;
    QVBoxLayout* layout_;
    void saveKeySettings();
};
