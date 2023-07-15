#include "mainwindow.hxx"
#include <log.hxx>
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char* argv[])
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(format);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    if (argc > 1)
    {
        w.OpenFile(argv[1]);
    }
    return a.exec();
}
