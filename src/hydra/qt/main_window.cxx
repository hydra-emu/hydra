#include <hydra/common/settings.hxx>
#include <hydra/common/translate.hxx>
#include <hydra/common/version.hxx>
#include <hydra/qt/main_window.hxx>
#include <hydra/resources/resources.hxx>
#include <hydra/sdl3/window.hxx>

#include <imgui.h>

#include <QDropEvent>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <thread>

std::vector<std::string> listToVector(const QStringList& list)
{
    std::vector<std::string> result;
    result.reserve(list.size());
    for (const QString& str : list)
    {
        result.push_back(str.toStdString());
    }
    return result;
}

namespace hydra::qt
{

    MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
    {
        setWindowTitle(QString::fromStdString(hydra::common::version()));
        setWindowIcon(hydra::resources::icon());
        setUnifiedTitleAndToolBarOnMac(true);
        setAcceptDrops(true);
        setAttribute(Qt::WA_NativeWindow);

        resize(settings::Config::get().windowWidth, settings::Config::get().windowHeight);

        std::thread t([this] {
            SDL3::Context* ctx = hydra::SDL3::Vk::init();
            while (1)
            {
                hydra::SDL3::Common::poll(ctx);
                hydra::SDL3::Vk::startFrame(ctx);
                ImGui::Begin("Hello, world!");
                ImGui::Button("Hello, world!");
                ImGui::End();
                hydra::SDL3::Vk::endFrame(ctx);
            }
        });
        t.detach();
    }

    MainWindow::~MainWindow() {}

    void MainWindow::dropEvent(QDropEvent* event)
    {
        const QList<QUrl>& urls = event->mimeData()->urls();
        if (urls.empty())
            return;

        QStringList files;
        QStringList folders;

        for (const QUrl& url : urls)
        {
            QFileInfo fileInfo(url.toLocalFile());
            QString path = fileInfo.filePath();

            if (!fileInfo.exists() || !fileInfo.isReadable())
            {
                QMessageBox::warning(this, htr("Error"), htr("Cannot read file: {}", path));
                return;
            }

            (fileInfo.isFile() ? files : folders).append(path);
        }

        if (!files.empty())
        {
            loadFiles(listToVector(files));
        }
        else
        {
            for (const QString& folder : folders)
            {
                if (QMessageBox::question(this, htr("Confirm"),
                                          htr("Add {} to list of game paths?", folder)) ==
                    QMessageBox::Yes)
                {
                    settings::Config::get().gameDirectories.push_back(folder.toStdString());
                }
            }
        }
    }

    void MainWindow::resizeEvent(QResizeEvent* event)
    {
        settings::Config::get().windowWidth = event->size().width();
        settings::Config::get().windowHeight = event->size().height();
    }

    void MainWindow::loadFiles(const std::vector<std::string>& files)
    {
        settings::Config::get().lastContentPaths = files;
    }
} // namespace hydra::qt