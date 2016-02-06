#version 330 core
#define PI 3.14159216
uniform sampler2D tex_color;
uniform int width;
uniform int height;

vec2 texel;

vec3 color(vec2 c) {
  return texture2D(tex_color, vec2(texel.x * c.x, texel.y * c.y)).rgb;
}

float gamma(float c) {
  float a = 0.055;
  if (c < 0.0031308) {
    return c * 12.92;
  } else {
    return (1 + a) * pow(c, 1.0/2.4) - a;
  }
}

void main () {
  texel = vec2(1.0 / float(width), 1.0 / float(height));
  vec3 c = color(gl_FragCoord.xy);
  gl_FragColor = vec4(gamma(c.r), gamma(c.g), gamma(c.b), 1.0);
}
