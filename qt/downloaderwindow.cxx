#include "downloaderwindow.hxx"
#include "download.hxx"
#include "hsystem.hxx"
#include "observer.hxx"
#include "update.hxx"
#include <cstdint>
#include <fmt/format.h>
#include <future>
#include <QFuture>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QtConcurrent/QtConcurrent>
#include <QTextEdit>
#include <QTreeWidget>
#include <QVBoxLayout>

class DownloadProgressBar : public QProgressBar, public hydra::Subject
{
public:
    explicit DownloadProgressBar(QWidget* parent = nullptr) : QProgressBar(parent)
    {
        setMinimum(0);
        setMaximum(100);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setTextVisible(false);
    }

    void Download(const std::string& url)
    {
        if (watcher_)
        {
            printf("Watcher already exists...?\n");
            exit(1);
        }

        setMinimum(0);
        setMaximum(0);

        auto func = [this](uint64_t current, uint64_t total) {
            return update_callback(current, total);
        };
        QFuture<hydra::HydraBufferWrapper> future =
            QtConcurrent::run(hydra::Downloader::DownloadProgress, url, func);
        watcher_ = new QFutureWatcher<hydra::HydraBufferWrapper>;
        watcher_->setFuture(future);
        connect(watcher_, &QFutureWatcher<hydra::HydraBufferWrapper>::finished, this,
                &DownloadProgressBar::download_finished);
        notify();
    }

    bool IsDownloading() const
    {
        return watcher_ != nullptr;
    }

private:
    void download_finished()
    {
        hydra::Updater::InstallCore(watcher_->result());
        watcher_->deleteLater();
        watcher_ = nullptr;
        setMinimum(0);
        setMaximum(100);
        setValue(100);
        notify();
    }

    bool update_callback(uint64_t current, uint64_t total)
    {
        setMinimum(0);
        if (current == 0 && total == 0)
        {
            setMaximum(0);
        }
        else
        {
            setMaximum(100);
            setValue((current * 100) / total);
        }
        return true;
    }

    QFutureWatcher<hydra::HydraBufferWrapper>* watcher_ = nullptr;
};

class DownloadButton : public QPushButton, public hydra::Observer
{
public:
    explicit DownloadButton(DownloadProgressBar* progress_bar, const std::string& url,
                            QWidget* parent = nullptr)
        : QPushButton("Download", parent), hydra::Observer(progress_bar), url_(url),
          progress_bar_(progress_bar)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setMaximumWidth(100);
        setMaximumHeight(20);
        connect(this, &QPushButton::clicked, this, &DownloadButton::download_clicked);
    }

private:
    void download_clicked()
    {
        progress_bar_->Download(url_);
    }

    void update() override
    {
        if (progress_bar_->IsDownloading())
        {
            setEnabled(false);
            setText("Downloading...");
        }
        else
        {
            setEnabled(true);
            setText("Download");
        }
    }

    DownloadProgressBar* progress_bar_ = nullptr;
    std::string url_;
};

DownloaderWindow::DownloaderWindow(QWidget* parent) : QWidget(parent, Qt::Window)
{
    QTreeWidget* tree = new QTreeWidget;
    tree->setHeaderLabels({"Cores"});
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tree->setAnimated(true);
    tree->setIndentation(20);
    tree->setColumnCount(1);

    uint32_t minimum_size = 0;
    auto database = hydra::Updater::GetDatabase();
    DownloadProgressBar* bar = new DownloadProgressBar;
    for (auto& [key, entries] : database)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, QString::fromStdString(key));
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        tree->addTopLevelItem(item);
        for (auto& entry : entries)
        {
            QTreeWidgetItem* child = new QTreeWidgetItem;
            QWidget* widget = new QWidget;
            QHBoxLayout* layout = new QHBoxLayout;
            QLabel* label =
                new QLabel(QString::fromStdString(entry.CoreName + " " + entry.CoreSubName));

            layout->addWidget(label);
            widget->setLayout(layout);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            widget->setMaximumHeight(25);
            widget->setContentsMargins(6, 0, 6, 0);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addStretch();

            uint32_t icons_size = 0;
            for (auto& [key, _] : entry.Downloads)
            {
                QLabel* label = new QLabel;
                std::string os = key.substr(0, key.find_first_of(' '));
                QString path = ":/images/" + QString::fromStdString(os) + ".png";
                label->setPixmap(QPixmap(path).scaled(16, 16, Qt::KeepAspectRatio));
                layout->addWidget(label);
                icons_size += 16;
            }

            DownloadButton* button = new DownloadButton(bar, entry.Downloads[hydra_os()]);
            layout->addWidget(button);

            item->addChild(child);
            tree->setItemWidget(child, 0, widget);

            uint32_t new_size = label->sizeHint().width() + button->sizeHint().width() + icons_size;
            if (new_size > minimum_size)
                minimum_size = new_size + 20;
        }
    }

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(tree);
    layout->addWidget(bar);
    setLayout(layout);

    setMinimumSize(minimum_size + 100, 0);
    setWindowTitle("Core Downloader");

    show();
}

DownloaderWindow::~DownloaderWindow() {}
