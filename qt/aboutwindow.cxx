#include "aboutwindow.hxx"
#include <error_factory.hxx>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

AboutWindow::AboutWindow(QWidget* parent) : QDialog(parent)
{
    static QString html;
    if (html.isEmpty())
    {
        QFile file(":/about.html");
        if (file.open(QIODevice::ReadOnly))
        {
            html = file.readAll();
        }
        else
        {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to load about.html");
        }
    }
    setWindowTitle("About");
    QGridLayout* layout = new QGridLayout;
    QHBoxLayout* top_layout = new QHBoxLayout;
    QHBoxLayout* bot_layout = new QHBoxLayout;
    QLabel* hydra = new QLabel;
    QLabel* logo = new QLabel;
    QLabel* lbl_text = new QLabel;
    QGroupBox* top_qgb = new QGroupBox;
    QGroupBox* bot_qgb = new QGroupBox;
    top_qgb->setFlat(true);
    bot_qgb->setFlat(true);
    top_qgb->setStyleSheet("border:0;");
    hydra->setPixmap(QPixmap::fromImage(QImage(":/images/hydra.png")));
    logo->setPixmap(QPixmap::fromImage(QImage(":/images/logo.png")));
    lbl_text->setText(html);
    lbl_text->setTextFormat(Qt::RichText);
    lbl_text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    lbl_text->setOpenExternalLinks(true);
    lbl_text->setAlignment(Qt::AlignCenter);
    layout->setAlignment(Qt::AlignLeft);
    layout->setContentsMargins(5, 5, 5, 5);
    top_layout->addWidget(hydra);
    top_layout->addWidget(logo);
    bot_layout->addWidget(lbl_text);
    top_qgb->setLayout(top_layout);
    bot_qgb->setLayout(bot_layout);
    layout->addWidget(top_qgb, 0, 0);
    layout->addWidget(bot_qgb, 1, 0);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(layout);
    show();
}
