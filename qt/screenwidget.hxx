#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H
#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QResizeEvent>
#include <QString>

class ScreenWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    ScreenWidget(QWidget* parent = nullptr);
    ~ScreenWidget();
    void Redraw(int width, int height, const void* data);

    void SetMouseMoveCallback(std::function<void(QMouseEvent*)> callback)
    {
        mouse_move_callback_ = callback;
    }

    unsigned GetFbo()
    {
        return fbo_;
    }

    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    GLuint texture_ = 0;
    GLuint fbo_ = 0;
    bool initialized_ = false;
    int current_width_ = 0;
    int current_height_ = 0;

    std::function<void(QMouseEvent*)> mouse_move_callback_;

    friend class MainWindow;
};
#endif