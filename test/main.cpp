#include <QApplication>
#include "SimpleGLWindow.h"
#include "SimpleGLScene.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    SimpleGLScene scene;
    SimpleGLWindow w(&scene);
    w.show();
    return a.exec();
}
