#include "SimpleGLWindow.h"

#include "GLScene.h"
// #include "glassert.h"

#include <iostream>
#include <QOpenGLContext>
#include <QTimer>

static void infoGL()
{
    const GLubyte *str;
    std::cout << "OpenGL infos with gl functions" << std::endl;
    str = glGetString(GL_RENDERER);
    std::cout << "Renderer : " << str << std::endl;
    str = glGetString(GL_VENDOR);
    std::cout << "Vendor : " << str << std::endl;
    str = glGetString(GL_VERSION);
    std::cout << "OpenGL Version : " << str << std::endl;
    str = glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::cout << "GLSL Version : " << str << std::endl;
}

SimpleGLWindow::SimpleGLWindow(GLScene *a_scene, QScreen *screen) : QWindow(screen), scene(a_scene) {
    setSurfaceType(OpenGLSurface);
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);

    setFormat(format);
    create();

    context = new QOpenGLContext();
    context->setFormat(format);
    context->create();

    scene->setContext(context);

    printContextInfos();
    initGL();

    resize(QSize(800, 450));

    connect(this, SIGNAL(widthChanged(int)), this, SLOT(resizeGL()));
    connect(this, SIGNAL(heightChanged(int)), this, SLOT(resizeGL()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateScene()));
    timer->start(16);
}

SimpleGLWindow::~SimpleGLWindow()
{
}

void SimpleGLWindow::printContextInfos()
{
    if(!context->isValid())
        std::cerr << "The OpenGL context is invalid!" << std::endl;

    context->makeCurrent(this);

    std::cout << "SimpleGLWindow format version is: "
              << format().majorVersion() << "."
              << format().minorVersion() << std::endl;

    std::cout << "Context format version is: "
              << context->format().majorVersion()
              << "." << context->format().minorVersion() << std::endl;
    infoGL();
}

void SimpleGLWindow::initGL() {
    context->makeCurrent(this);
    scene->init();
}

void SimpleGLWindow::paintGL() {
    //if( !isExposed() ) return;
    context->makeCurrent(this);
    scene->render();
    context->swapBuffers(this);
}

void SimpleGLWindow::resizeGL() {
    context->makeCurrent(this);
    scene->resize(width(), height());
}

void SimpleGLWindow::updateScene() {
    scene->update();
    paintGL();
}
