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
    DownloaderWindow(const std::vector<std::string>& download_queue);
    ~DownloaderWindow();

private:
    std::vector<std::string> download_queue_;
    bool downloading_ = false;

    QProgressBar* progress_bar_;
    QTextEdit* log_;
    QLabel* byte_progress_label_;
};