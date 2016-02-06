#ifndef SIMPLE_GL_SCENE_H
#define SIMPLE_GL_SCENE_H
#include <armadillo>
#include "GLScene.h"
#include "OpenGL++11.h"
#include "geom.h"
#include <yaml-cpp/yaml.h>

class SimpleGLScene : public GLScene {
public:
  SimpleGLScene();

  virtual void init();
  virtual void update();
  virtual void render();
  virtual void resize(int width, int height);

private:
  OpenGL11::ShaderProgram shader, postprocess, blur;
  OpenGL11::VertexArray vao;
  OpenGL11::Texture2D renderedColorTexture, renderedDepthTexture;
  OpenGL11::Framebuffer framebuffer;
  OpenGL11::Buffer<GLfloat> stripBuffer, quadBuffer;
  geom::ftransform camera;
  OpenGL11::fmat4 view, projection;
  uint64_t t0;
  YAML::Node sceneNode;

  void initShaders();
  void initBuffers();
  int sceneWidth = 0, sceneHeight = 0;
};

#endif // SIMPLE_GL_SCENE_H
