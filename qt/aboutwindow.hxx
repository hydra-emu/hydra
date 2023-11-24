#pragma once

#include <QDialog>

class AboutWindow : public QDialog
{
    Q_OBJECT

public:
    AboutWindow(QWidget* parent = nullptr);
    ~AboutWindow() = default;
};
