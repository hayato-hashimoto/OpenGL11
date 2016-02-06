#include "SimpleGLScene.h"
#include "Projection.h"
#include <GLXW/glxw.h>
#include <GL/gl.h>
#include <array>
#include <iostream>
#include <QDateTime>
#include <yaml-cpp/yaml.h>
#include "geom.h"

SimpleGLScene::SimpleGLScene()
    : shader(),
      postprocess(),
      blur(),
      vao(),
      renderedColorTexture(GL_RGBA8),
      renderedDepthTexture(GL_DEPTH_COMPONENT24),
      framebuffer(),
      stripBuffer(GL_ARRAY_BUFFER),
      quadBuffer(GL_ARRAY_BUFFER),
      camera() {}

void SimpleGLScene::init() {
  t0 = QDateTime::currentMSecsSinceEpoch();
  glxwInit();
  glGetError(); // read & ignore GL_INVALID_ENUM here (GLEW bug)
  initShaders();
  initBuffers();
  glEnable(GL_DEPTH_TEST);
  sceneNode = YAML::LoadFile("test/scene.yaml");
}

void SimpleGLScene::update() {
  // miliseconds from init()
  float uptime = QDateTime::currentMSecsSinceEpoch() - t0;
  float alpha  = 0.6 - 0.5 * sin(M_PI * 0.00005 * uptime), beta = 0.0002 * uptime, r = 30.0 - 20.0 * sin(M_PI * 0.00005 * uptime);
  camera = geom::translate(0.0f, 0.0f, -r) * geom::rotate(0.0f, alpha, beta);
}

void SimpleGLScene::render() {
  // make sure SimpleGLScene::resize() is called (and the textures are ready).
  if (!(sceneWidth * sceneHeight)) { return; }
  OpenGL11::fmat4 model;
  framebuffer.bind();
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL_CHECK_ERROR();
  view = camera;
  for (size_t i = 0; i < sceneNode.size(); i++) {
    YAML::Node node_type = sceneNode[i];
    YAML::Node node;
    if (node_type["helix"]) {
      node = node_type["helix"];
      std::vector<float> p = (node["position"].as<std::vector<float>>());
      model = geom::translate(p);
      shader.bind(vao,
          "pos",         stripBuffer,
          "program",     1,
          "r",           node["r"].as<GLfloat>(),
          "width",       node["width"].as<GLfloat>(),
          "num_v",       512.0f,
          "angle",       node["angle"].as<GLfloat>(),
          "helix_angle", node["helix_angle"].as<GLfloat>(),
          "proj",        projection,
          "view",        view,
          "model",       model);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 512);
      GL_CHECK_ERROR();
    } else if (node_type["line"]) {
      node = node_type["line"];
      std::vector<GLfloat> p = (node["position"].as<std::vector<GLfloat>>());
      shader.bind(vao,
          "pos",         stripBuffer,
          "program",     0,
          "width",       node["width"].as<GLfloat>(),
          "len",         node["len"].as<GLfloat>(),
          "num_v",       512.0f,
          "proj",        projection,
          "view",        view,
          "model",       model);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 512);
      GL_CHECK_ERROR();
    } else if (node_type["clothoid"]) {
      node = node_type["clothoid"];
      std::vector<GLfloat> p = (node["position"].as<std::vector<GLfloat>>());
      shader.bind(vao,
          "pos",         stripBuffer,
          "program",     2,
          "width",       node["width"].as<GLfloat>(),
          "angle",       node["angle"].as<GLfloat>(),
          "slope_angle", node["slope_angle"].as<GLfloat>(),
          "len",         node["len"].as<GLfloat>(),
          "num_v",       512.0f,
          "proj",        projection,
          "view",        view,
          "model",       model);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 512);
      GL_CHECK_ERROR();
    }
  }
  GL_CHECK_ERROR();
  //framebuffer.detach(GL_DEPTH_ATTACHMENT);
  glDisable(GL_DEPTH_TEST);
  blur.bind(vao,
      "pos",     quadBuffer,
      "tex_color", renderedColorTexture,
      "tex_depth", renderedDepthTexture,
      "width",   sceneWidth,
      "height",  sceneHeight,
      "iter",    0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  blur.bind(vao, "iter", 1);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  blur.bind(vao, "iter", 2);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  blur.bind(vao, "iter", 3);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  /*
  blur.bind(vao, "iter", 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  blur.bind(vao, "iter", 5);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  */
  framebuffer.unbind();

  glClear(GL_COLOR_BUFFER_BIT);
  postprocess.bind(vao,
      "pos",     quadBuffer,
      "tex_color", renderedColorTexture,
      "width",   sceneWidth,
      "height",  sceneHeight);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  GL_CHECK_ERROR();
}

void SimpleGLScene::resize(int width, int height) {
 sceneWidth = width, sceneHeight = height;
 glViewport(0, 0, width, height);
 projection = mat_perspective(60, width / (double) height, 1.0, 200.0);
 renderedColorTexture.allocate(width, height);
 renderedColorTexture.setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 renderedColorTexture.setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 renderedColorTexture.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
 renderedColorTexture.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 renderedDepthTexture.allocate(width, height);
 renderedDepthTexture.setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 renderedDepthTexture.setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 renderedDepthTexture.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
 renderedDepthTexture.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 framebuffer.attach(
     GL_COLOR_ATTACHMENT0, renderedColorTexture,
     GL_DEPTH_ATTACHMENT,  renderedDepthTexture);
 GL_CHECK_ERROR();
}

void SimpleGLScene::initShaders() {
  shader.link("test/shaders/helix.vert", "test/shaders/helix.frag");
  GL_CHECK_ERROR();
  blur.link("test/shaders/postprocess.vert", "test/shaders/kawase.frag");
  GL_CHECK_ERROR();
  postprocess.link("test/shaders/postprocess.vert", "test/shaders/gamma.frag");
  GL_CHECK_ERROR();
  framebuffer.create();
  GL_CHECK_ERROR();
}

// render points with their coordinate
// (x, y) = (0, 0), (1, 0), ..., (x_size, 0), (0, 1), ..., (x_size, y_size)
template <typename T>
void initMeshArray (std::vector<T> &data, int x_size, int y_size) {
  for (int i = 0; i < y_size; i++) {
    for (int j = 0; j < x_size; j++) {
      data[2 * (i * x_size + j)] = j;
      data[2 * (i * x_size + j) + 1] = i;
    }
  }
}

void SimpleGLScene::initBuffers() {
  int strip_x = 2, strip_y = 512;
  std::vector<float> stripData(2 * strip_x * strip_y), quadData(2 * 2 * 2), lineData = { 0, 1, 0, 2, 0, 3 }, gridData(64 * 64 * 2);
  initMeshArray(stripData, strip_x, strip_y);
  initMeshArray(quadData, 2, 2);
  initMeshArray(gridData, 64, 64);
  stripBuffer.allocate(stripData, 2);
  quadBuffer.allocate(quadData, 2);
  vao.create();
  GL_CHECK_ERROR();
}
