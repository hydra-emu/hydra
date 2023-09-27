#include "mainwindow.hxx"
#include <log.h>
#include <QApplication>
#include <QSurfaceFormat>
#include <settings.hxx>

int main(int argc, char* argv[])
{
    Settings::InitCoreInfo();
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

    int ret = 0;
    ret = a.exec();
    try
    {
    } catch (...)
    {
        log_fatal("Caught unhandled exception. Please open an issue on Github.");
        return 1;
    }
    return ret;
}
