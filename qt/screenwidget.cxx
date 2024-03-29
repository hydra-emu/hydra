#include "screenwidget.hxx"
#include <iostream>
#include <log.h>
#include <QFile>
#include <QSurfaceFormat>

ScreenWidget::ScreenWidget(QWidget* parent) : QOpenGLWidget(parent) {}

ScreenWidget::~ScreenWidget()
{
    if (initialized_)
    {
        if (texture_ != 0)
            glDeleteTextures(1, &texture_);
        if (fbo_ != 0)
            glDeleteFramebuffers(1, &fbo_);
    }
}

void ScreenWidget::Redraw(void* tdata)
{
    if (initialized_) [[likely]]
    {
        if (tdata)
        {
            // TODO: find a faster way to do this (set all alphas to max or ignore alpha - otherwise
            // causes windows to be transparent on wayland)
            for (int i = 3; i < current_width_ * current_height_ * 4; i += 4)
            {
                ((uint8_t*)tdata)[i] = 0xFF;
            }
            // Flip texture upside down when copying
            glBindTexture(GL_TEXTURE_2D, texture_);
            for (int i = 0; i < current_height_; i++)
            {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, current_height_ - i - 1, current_width_, 1,
                                GL_RGBA, GL_UNSIGNED_BYTE,
                                static_cast<const uint8_t*>(tdata) + current_width_ * 4 * i);
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        update();
    }
}

void ScreenWidget::Resize(int width, int height)
{
    if (initialized_)
    {
        if (width != current_width_ || height != current_height_)
        {
            setFixedWidth(width);
            setFixedHeight(height);
            current_width_ = width;
            current_height_ = height;
            if (texture_ != 0)
                glDeleteTextures(1, &texture_);
            glGenTextures(1, &texture_);
            glBindTexture(GL_TEXTURE_2D, texture_);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            if (fbo_ != 0)
                glDeleteFramebuffers(1, &fbo_);
            glGenFramebuffers(1, &fbo_);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_,
                                   0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            update();
            // Check if fbo is complete
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE)
            {
                printf("Framebuffer not complete: %x\n", status);
                log_fatal("Framebuffer not complete, stopping\n");
                return;
            }
        }
    }
    else
    {
        printf("ScreenWidget not initialized\n");
    }
}

void ScreenWidget::mouseMoveEvent(QMouseEvent* event)
{
    mouse_move_callback_(event);
}

void ScreenWidget::mousePressEvent(QMouseEvent* event)
{
    mouse_click_callback_(event);
}

void ScreenWidget::mouseReleaseEvent(QMouseEvent* event)
{
    mouse_release_callback_(event);
}

void ScreenWidget::initializeGL()
{
    initializeOpenGLFunctions();
    hide();
    initialized_ = true;
}

void ScreenWidget::resizeGL(int, int) {}

void ScreenWidget::paintGL()
{
    if (initialized_)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
        glBlitFramebuffer(0, 0, current_width_, current_height_, 0, 0, current_width_,
                          current_height_, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
}
