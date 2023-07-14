#pragma once

#include <QDialog>

class AboutWindow : public QDialog
{
    Q_OBJECT
  private:
    bool& open_;

  public:
    AboutWindow(bool& open, QWidget* parent = nullptr);
    ~AboutWindow();
};
