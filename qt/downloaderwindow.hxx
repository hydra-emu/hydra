#pragma once

#include <QWidget>
#include <string>
#include <vector>

class QProgressBar;
class QTextEdit;
class QLabel;

class DownloaderWindow : public QWidget
{
    Q_OBJECT

public:
    DownloaderWindow(QWidget* parent = nullptr);
    ~DownloaderWindow();
};