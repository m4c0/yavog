#version 450

layout(push_constant) uniform upc {
  vec2 angles;
} pc;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 normal;

layout(location = 0) out vec4 f_normal;

const float f = 1.0 / tan(radians(90) / 2.0);
const float far = 10;
const float near = 0.1;

void main() {
  float ax = radians(pc.angles.x);
  float ay = radians(pc.angles.y);
  vec4 p = mat4(
    cos(ax), 0, sin(ax), 0,
    0, 1, 0, 0,
    -sin(ax), 0, cos(ax), 0,
    0, 0, 0, 1
  ) * mat4(
    1, 0, 0, 0,
    0, cos(ay), sin(ay), 0,
    0, -sin(ay), cos(ay), 0,
    0, 0, 0, 1
  ) * pos + vec4(0, 0, 2, 0);
  gl_Position = mat4(
    f, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -(far * near) / (far - near), 0
  ) * p;
  gl_PointSize = 20;
  f_normal = normal;
}
