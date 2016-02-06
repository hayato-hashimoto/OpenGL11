#include "OpenGL++11.h"
#include "Projection.h"
OpenGL11::fmat4 mat_perspective(float fov, float ratio, float nearP, float farP) {
  OpenGL11::fmat4 out;
  float f = 1.0f / tan (fov * (M_PI / 360.0));
  out.eye();
  out(0, 0) = f / ratio;
  out(1, 1) = f;
  out(2, 2) = (farP + nearP) / (nearP - farP);
  out(2, 3) = (2.0f * farP * nearP) / (nearP - farP);
  out(3, 2) = -1.0f;
  out(3, 3) = 0.0f;
  return out;
}
