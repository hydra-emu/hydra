#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QResizeEvent>

class ScreenWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    ScreenWidget(QWidget *parent = nullptr);
    ~ScreenWidget();
    void InitializeTexture(int width, int height, void* data);
    void Redraw(int width, int height, void* data);
private:
    void initializeGL() override;
    void paintGL() override;
    GLuint texture_;
    QOpenGLShaderProgram* program_ = nullptr;
    QOpenGLBuffer vbo_;
    bool initialized_ = false;
};
#endif