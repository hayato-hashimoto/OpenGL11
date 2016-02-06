QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt5-opengl11-test
TEMPLATE = app
QMAKE_CXX = gcc
QMAKE_CXXFLAGS += -std=c++11 -g
LIBS += -lyaml-cpp -ldl -larmadillo
INCLUDEPATH += src/ include/ deps/lodepng
LIBPATH += deps/glxw/

SOURCES += \
    src/glxw.c \
    test/main.cpp \
    test/SimpleGLWindow.cpp \
    test/SimpleGLScene.cpp \
    test/Projection.cpp \
    deps/lodepng/lodepng.cpp

HEADERS += \
    src/OpenGL++11.h \
    test/geom.h \
    test/SimpleGLWindow.h \
    test/GLScene.h \
    test/SimpleGLScene.h \
    test/Projection.h \

DEFINES += \
USE_ARMADILLO

glxw.target = \
    src/glxw.c

glxw.commands = \
    python deps/glxw/glxw_gen.py

QMAKE_EXTRA_TARGETS += glxw
