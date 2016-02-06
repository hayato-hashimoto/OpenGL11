#version 130
uniform int width;
uniform int height;
uniform float len;
attribute vec2 pos;
void main() {
  gl_Position = vec4(2.0 * pos.x -1.0, 2.0 * pos.y - 1.0, 1.0, 1.0);
}
