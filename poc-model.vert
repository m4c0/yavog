#version 450

layout(push_constant) uniform upc {
  float angle;
} pc;

layout(location = 0) in vec4 pos;

const float f = 1.0 / tan(radians(90) / 2.0);
const float far = 10;
const float near = 0.1;

void main() {
  float a = radians(pc.angle);
  vec4 p = mat4(
    cos(a), 0, sin(a), 0,
    0, 1, 0, 0,
    -sin(a), 0, cos(a), 0,
    0, 0, 0, 1
  ) * pos + vec4(0, 0, 2, 0);
  gl_Position = mat4(
    f, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -(far * near) / (far - near), 0
  ) * p;
  gl_PointSize = 20;
}
