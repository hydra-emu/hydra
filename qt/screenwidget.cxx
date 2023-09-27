#include "screenwidget.hxx"
#include <iostream>
#include <log.h>
#include <QFile>
#include <QSurfaceFormat>

// clang-format off

static constexpr float vertices_uvs[] =
{
  -1,  1, 0, 1,
   1,  1, 1, 1,
   1, -1, 1, 0,
   1, -1, 1, 0,
  -1, -1, 0, 0,
  -1,  1, 0, 1,
};

// clang-format on

ScreenWidget::ScreenWidget(std::function<void(unsigned)> set_fbo_callback, QWidget* parent)
    : set_fbo_callback_(set_fbo_callback), QOpenGLWidget(parent),
      vbo_(QOpenGLBuffer::Type::VertexBuffer)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ScreenWidget::~ScreenWidget()
{
    if (initialized_)
    {
        glDeleteTextures(1, &texture_);
    }
    delete program_;
}

void ScreenWidget::Redraw(int width, int height, const void* tdata)
{
    if (initialized_) [[likely]]
    {
        if (width != current_width_ || height != current_height_)
        {
            current_width_ = width;
            current_height_ = height;
            if (texture_ != 0)
                glDeleteTextures(1, &texture_);
            glGenTextures(1, &texture_);
            glBindTexture(GL_TEXTURE_2D, texture_);
            // glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            if (fbo_ != 0)
                glDeleteFramebuffers(1, &fbo_);
            glGenFramebuffers(1, &fbo_);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_,
                                   0);
            set_fbo_callback_(fbo_);
        }
        if (tdata)
        {
            glBindTexture(GL_TEXTURE_2D, texture_);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                            tdata);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        update();
    }
}

void ScreenWidget::ResetProgram(QString* vertex, QString* fragment)
{
    if (!vertex)
    {
        QFile vfile(":/shaders/simple.vs");
        vfile.open(QIODevice::ReadOnly);
        vshader_source_ = vfile.readAll();
    }
    else
    {
        vshader_source_ = *vertex;
    }
    if (!fragment)
    {
        QFile ffile(":/shaders/simple.fs");
        ffile.open(QIODevice::ReadOnly);
        fshader_source_ = ffile.readAll();
    }
    else
    {
        fshader_source_ = *fragment;
    }
    QOpenGLShader* vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    vshader->compileSourceCode(vshader_source_);
    if (!vshader->log().isEmpty())
    {
        auto error = vshader->log().toStdString();
        delete vshader;
        throw std::runtime_error(error);
    }
    QOpenGLShader* fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    fshader->compileSourceCode(fshader_source_);
    if (!fshader->log().isEmpty())
    {
        auto error = fshader->log().toStdString();
        delete fshader;
        throw std::runtime_error(error);
    }
    if (program_)
    {
        delete program_;
    }
    program_ = new QOpenGLShaderProgram;
    program_->addShader(vshader);
    program_->addShader(fshader);
    program_->link();
    program_->bind();
    glActiveTexture(GL_TEXTURE0);
    GLint tex = glGetUniformLocation(program_->programId(), "tex");
    if (tex == -1)
    {
        log_fatal("Could not find uniform tex");
    }
    glUniform1i(tex, 0);
    delete fshader;
    delete vshader;
}

void ScreenWidget::mouseMoveEvent(QMouseEvent* event)
{
    mouse_move_callback_(event);
}

void ScreenWidget::initializeGL()
{
    initializeOpenGLFunctions();
    vao_.create();
    vao_.bind();
    vbo_.create();
    glBindBuffer(GL_ARRAY_BUFFER, vbo_.bufferId());
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_uvs), vertices_uvs, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnable(GL_TEXTURE_2D);
    ResetProgram();
    initialized_ = true;
}

void ScreenWidget::resizeGL(int, int) {}

void ScreenWidget::paintGL()
{
    program_->bind();
    if (initialized_)
    {
        glBindTexture(GL_TEXTURE_2D, texture_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    program_->release();
}