#version 330

in vec4 normal;
in vec3 c;

void main(void) {
  gl_FragColor.a = 1.0;
  gl_FragColor.rgb = dot(normal, vec4(cos(1.0), sin(1.0), 0.0, 0.0)) * c;
}
