#version 330 core
#define PI 3.14159216
uniform sampler2D tex_color;
uniform sampler2D tex_depth;
uniform int width;
uniform int height;
uniform int iter;

vec2 texel;

uint jenkins_lookup3_hash (uvec3 s) {
  uint a = s.x, b = s.y, c = s.z;
  #define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
  a -= c;  a ^= rot(c, 4);  c += b;
  b -= a;  b ^= rot(a, 6);  a += c;
  c -= b;  c ^= rot(b, 8);  b += a;
  a -= c;  a ^= rot(c,16);  c += b;
  b -= a;  b ^= rot(a,19);  a += c;
  c -= b;  c ^= rot(b, 4);  b += a;
  return c;
}

vec3 color(vec2 c) {
  return texture2D(tex_color, vec2(texel.x * c.x, texel.y * c.y)).rgb;
}

float depth(vec2 c) {
  return texture2D(tex_depth, vec2(texel.x * c.x, texel.y * c.y)).r;
}

float dof_factor(float z, float focus, float depth) {
  return abs(1.0/z - 1.0/focus) / (1.0 / (focus + depth) - 1.0 / focus);
}

float dof_modifier(float f) {
  return f * f;
}

void main () {
  int i = iter;
  texel = vec2(1.0 / float(width), 1.0 / float(height));
  vec2 coord = gl_FragCoord.xy;
  ivec2 icoord = ivec2(coord);
  int j = i;

  float dxy = float(i) + 0.7;
  float f1 = min(0.25, 0.5 * iter * dof_modifier(dof_factor(depth(coord + vec2(dxy-j,  dxy   )), 0.92, 0.2))),
        f2 = min(0.25, 0.5 * iter * dof_modifier(dof_factor(depth(coord + vec2(-dxy,   dxy-j )), 0.92, 0.2))),
        f3 = min(0.25, 0.5 * iter * dof_modifier(dof_factor(depth(coord + vec2(-dxy+j, -dxy  )), 0.92, 0.2))),
        f4 = min(0.25, 0.5 * iter * dof_modifier(dof_factor(depth(coord + vec2(dxy,    -dxy+j)), 0.92, 0.2)));

  dxy = float(i) + 0.5;
  gl_FragColor.rgb =
    f1 * color(coord + vec2(dxy-j, dxy)) +
    f2 * color(coord + vec2(-dxy, dxy-j)) +
    f3 * color(coord + vec2(-dxy+j, -dxy)) +
    f4 * color(coord + vec2(dxy, -dxy+j)) +
    (1.0 - f1 - f2 - f3 - f4) * color(coord);
  gl_FragColor.a = 1.0;
}
