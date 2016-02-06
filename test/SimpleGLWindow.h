#ifndef SIMPLE_GL_WINDOW_H
#define SIMPLE_GL_WINDOW_H

#include "GLScene.h"

#include <QWindow>

class QOpenGLContext;

class SimpleGLWindow : public QWindow
{
    Q_OBJECT
public:
    SimpleGLWindow(GLScene* scene, QScreen *screen = 0);
    ~SimpleGLWindow();
    
signals:
    
public slots:

protected slots:
    void resizeGL();
    void paintGL();
    void updateScene();

private:
    void printContextInfos();

    QOpenGLContext *context;
    GLScene *scene;
    
    void initGL();
};

#endif // SIMPLE_GL_WINDOW_H
