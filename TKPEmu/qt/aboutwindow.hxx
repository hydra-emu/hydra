#pragma once
#ifndef TKP_ABOUT_H
#define TKP_ABOUT_H
#include <QDialog>

class AboutWindow : public QDialog {
    Q_OBJECT
private:
    bool& open_;
public:
    AboutWindow(bool& open, QWidget* parent = nullptr);
    ~AboutWindow();

};
#endif