#include "screenwidget.hxx"
#include <iostream>
#include <QFile>

bool ScreenWidget::GLInitialized = false;

ScreenWidget::ScreenWidget(QWidget *parent) : QOpenGLWidget(parent) {}

ScreenWidget::~ScreenWidget() {
    vbo_.destroy();
    delete program_;
    glDeleteTextures(1, &texture_);
}

void ScreenWidget::InitializeTexture(int width, int height, void* data) {
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    initialized_ = true;
}

void ScreenWidget::Redraw(int width, int height, void* data) {
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ScreenWidget::ResetProgram(QString* vertex, QString* fragment) {
    
    if (!vertex) {
        QFile vfile(":/shaders/simple.vs"); vfile.open(QIODevice::ReadOnly);
        vshader_source_ = vfile.readAll();
    } else {
        vshader_source_ = *vertex;
    }
    if (!fragment) {
        QFile ffile(":/shaders/simple.fs"); ffile.open(QIODevice::ReadOnly);
        fshader_source_ = ffile.readAll();
    } else {
        fshader_source_ = *fragment;
    }
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    vshader->compileSourceCode(vshader_source_);
    if (!vshader->log().isEmpty()) {
        auto error = vshader->log().toStdString();
        delete vshader;
        throw std::runtime_error(error);
    }
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    fshader->compileSourceCode(fshader_source_);
    if (!fshader->log().isEmpty()) {
        auto error = fshader->log().toStdString();
        delete fshader;
        throw std::runtime_error(error);
    }
    if (program_) {
        delete program_;
    }
    program_ = new QOpenGLShaderProgram;
    program_->addShader(vshader);
    program_->addShader(fshader);
    program_->link();
    program_->bind();
    glActiveTexture(GL_TEXTURE0);
    program_->setUniformValue("tex", 0);
    // program_->removeShader(fshader);
    delete fshader;
    delete vshader;
}

void ScreenWidget::initializeGL() {
    initializeOpenGLFunctions();
    glGenTextures(1, &texture_);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    ResetProgram();
}

void ScreenWidget::resizeGL(int width, int height) {
    
}

void ScreenWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    program_->bind();
    if (initialized_)
    {
        float vertices[] = {-1.0, 1.0, 1.0, 1.0, 1, -1, -1, -1};
        float coordTexture[] = {0, 0, 1, 0, 1, 1, 0, 1};
        GLint vertexLocation = glGetAttribLocation(program_->programId(), "in_Vertex");
        GLint texcoordLocation = glGetAttribLocation(program_->programId(), "vertTexCoord");
        glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(vertexLocation);
        glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, 0, coordTexture);
        glEnableVertexAttribArray(texcoordLocation);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glDrawArrays(GL_QUADS, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisableVertexAttribArray(texcoordLocation);
        glDisableVertexAttribArray(vertexLocation);
    }
    program_->release();
}