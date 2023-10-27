#include "downloaderwindow.hxx"
#include "download.hxx"
#include <fmt/format.h>
#include <future>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QVBoxLayout>

DownloaderWindow::DownloaderWindow(const std::vector<std::string>& download_queue)
    : QWidget(nullptr, Qt::Window), download_queue_(download_queue)
{
    setMinimumSize(500, 400);
    setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout* layout = new QVBoxLayout;
    log_ = new QTextEdit;
    log_->setReadOnly(true);
    log_->setLineWrapMode(QTextEdit::NoWrap);
    log_->setMinimumSize(500, 400);
    log_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    progress_bar_ = new QProgressBar;
    progress_bar_->setMinimum(0);
    progress_bar_->setMaximum(100);
    progress_bar_->setValue(0);

    byte_progress_label_ = new QLabel("");
    byte_progress_label_->setAlignment(Qt::AlignCenter);

    layout->addWidget(log_);
    layout->addWidget(byte_progress_label_);
    layout->addWidget(progress_bar_);
    setLayout(layout);

    setWindowFlags(Qt::WindowStaysOnTopHint);

    downloading_ = true;
    std::thread t([this]() {
        std::future<hydra::HydraBufferWrapper> f = std::async(std::launch::async, [this]() {
            auto ret = hydra::Downloader::DownloadProgress(
                download_queue_[0], [this](uint64_t current, uint64_t total) {
                    if (current == 0 && total == 0)
                    {
                        printf("Unknown size\n");
                    }
                    else
                    {
                        int percent = (int)(current * 100 / total);
                        printf("Progress: %d%%\n", percent);
                    }
                    return downloading_;
                });
            return ret;
        });
        f.wait();
    });
    t.detach();
}

DownloaderWindow::~DownloaderWindow() {}
