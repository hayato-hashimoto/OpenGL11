#version 330
 
uniform mat4 view, proj, model;

uniform float r, width, helix_angle, num_v, angle, len, slope_angle;
uniform int program;
in vec2 pos;
out vec4 normal;
out vec3 c;

void helix() {
  vec4 tmp;
  float phi = 0.4;
  float x = (2.0 * angle / num_v) * pos.y, y = pos.x;
  tmp.x = r * cos(x);
  tmp.y = r * x * tan(helix_angle) + width * y;
  tmp.z = r * sin(x);
  tmp.w = 1.0;
  gl_Position = proj * view * model * tmp;
  phi = phi * (-1.0 + 2.0 * y);
  normal = model * vec4(sin(phi) * cos(x), cos(phi), sin(phi) * sin(x), 0.0);
  tmp = view * model * tmp;
  c = vec3(1.0 - x / angle, x / angle, y);
}

void line() {
  vec4 tmp;
  float x, y;
  float phi = 0.4;
  x = (2.0 * len / num_v) * pos.y;
  y = pos.x;
  tmp[0] = x;
  tmp[1] = 0.0;
  tmp[2] = width * y;
  tmp[3] = 1.0;
  gl_Position = proj * view * model * tmp;
  phi = phi * (-1.0 + 2.0 * y);
  normal = model * vec4(sin(phi), cos(phi), 0.0, 0.0);
  tmp = view * model * tmp;
  c = vec3(1.0, 0.0, 0.0);
}

void clothoid() {
  vec4 tmp;
  float x, y, a, p, slope_a, q, z;
  x = (2.0 * len / num_v) * pos.y;
  y = pos.x;
  a = sqrt(angle) / len;
  slope_a = -sqrt(slope_angle)/len;
  p = a * x;
  q = slope_a * x;
  z = x*q*q/3.0 - x*q*q*q*q*q*q*q/42.0;
  tmp[0] = x - x*p*p*p*p/10.0 + x*p*p*p*p*p*p*p*p/9.0/24.0 - y * width * sin(p*p);
  tmp[1] = z;
  tmp[2] = x*p*p/3.0 - x*p*p*p*p*p*p*p/42.0 + y * width * cos(p*p);
  tmp[3] = 1.0;
  gl_Position = proj * view * model * tmp;
  normal = vec4(0.0, 1.0, 0.0, 0.0);
  tmp = view * model * tmp;
  c = vec3(1.0);
}

void main() {
  if (program == 0) line();
  if (program == 1) helix();
  if (program == 2) clothoid();
}
