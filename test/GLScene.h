#ifndef ABSTRACTSCENE_H
#define ABSTRACTSCENE_H

class QOpenGLContext;

class GLScene {
public:
    GLScene() : context(0) {}
    virtual ~GLScene(){}

    void setContext(QOpenGLContext *a_context) { context = a_context; }
    QOpenGLContext* getContext() const { return context; }

    virtual void init() = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    virtual void resize(int width, int height) = 0;

protected:
    QOpenGLContext *context;
};

#endif // ABSTRACTSCENE_H
