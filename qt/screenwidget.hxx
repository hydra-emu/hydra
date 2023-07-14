#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QResizeEvent>
#include <QString>

class ScreenWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
  public:
    ScreenWidget(QWidget* parent = nullptr);
    ~ScreenWidget();
    void InitializeTexture(int width, int height, int bitdepth, void* data);
    void Redraw(int width, int height, int bitdepth, void* data);
    void ResetProgram(QString* vertex = nullptr, QString* fragment = nullptr);

  private:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    GLuint texture_;
    QOpenGLShaderProgram* program_ = nullptr;
    QString vshader_source_;
    QString fshader_source_;
    bool initialized_ = false;
};
#endif