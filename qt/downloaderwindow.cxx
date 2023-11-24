#include "downloaderwindow.hxx"
#include "download.hxx"
#include "update.hxx"
#include <fmt/format.h>
#include <future>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeWidget>
#include <QVBoxLayout>

DownloaderWindow::DownloaderWindow(QWidget* parent) : QWidget(parent, Qt::Window)
{
    QTreeWidget* tree = new QTreeWidget;
    tree->setHeaderLabels({"Cores"});
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tree->setAnimated(true);
    tree->setIndentation(20);
    tree->setColumnCount(1);

    auto database = hydra::Updater::GetDatabase();
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

            for (auto& [key, _] : entry.Downloads)
            {
                QLabel* label = new QLabel;
                std::string os = key.substr(0, key.find_first_of(' '));
                QString path = ":/images/" + QString::fromStdString(os) + ".png";
                label->setPixmap(QPixmap(path).scaled(16, 16, Qt::KeepAspectRatio));
                layout->addWidget(label);
            }

            QPushButton* button = new QPushButton("Download");
            button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            button->setMaximumWidth(100);
            button->setMaximumHeight(20);
            layout->addWidget(button);

            item->addChild(child);
            tree->setItemWidget(child, 0, widget);
        }
    }

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(tree);
    setLayout(layout);
}

DownloaderWindow::~DownloaderWindow() {}
