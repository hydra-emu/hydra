#include "screenwidget.hxx"
#include <iostream>
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

void ScreenWidget::initializeGL() {
    initializeOpenGLFunctions();
    glGenTextures(1, &texture_);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
        "#version 150 core\n"
        "in vec2 in_Vertex;\n"
        "in vec2 vertTexCoord;\n"
        "out vec2 fragTexCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = vec4(in_Vertex, 0.0, 1.0);\n"
        "    fragTexCoord = vertTexCoord;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
            "#version 150 core\n"
            "uniform sampler2D tex;\n"
            "in vec2 fragTexCoord;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = texture2D(tex,fragTexCoord);\n"
            "}\n";
    fshader->compileSourceCode(fsrc);
    program_ = new QOpenGLShaderProgram;
    program_->addShader(vshader);
    program_->addShader(fshader);
    program_->link();
    program_->bind();
    glActiveTexture(GL_TEXTURE0);
    program_->setUniformValue("tex", 0);
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