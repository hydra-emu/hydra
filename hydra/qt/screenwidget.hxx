#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QResizeEvent>
#include <QString>

class ScreenWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    ScreenWidget(QWidget *parent = nullptr);
    ~ScreenWidget();
    void InitializeTexture(int width, int height, void* data);
    void Redraw(int width, int height, void* data);
    void ResetProgram(QString* vertex = nullptr, QString* fragment = nullptr);
    static bool GLInitialized;
private:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    GLuint texture_;
    QOpenGLShaderProgram* program_ = nullptr;
    QOpenGLBuffer vbo_;
    QString vshader_source_;
    QString fshader_source_;
    bool initialized_ = false;
};
#endif