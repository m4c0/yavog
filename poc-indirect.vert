#version 450

layout(location = 0) in vec4 i_pos;
layout(location = 1) in vec4 i_rot;
layout(location = 2) in vec4 i_size;
layout(location = 3) in vec4 pos;
layout(location = 4) in vec3 normal;
layout(location = 5) in vec2 uv;

layout(location = 0) out vec4 f_colour;

const float f = 1.0 / tan(radians(90) / 2.0);
const float far = 10;
const float near = 0.1;

void main() {
  vec4 p = pos + i_pos;

  p.xy *= -1;
  gl_Position = mat4(
    f, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -(far * near) / (far - near), 0
  ) * p;
  gl_PointSize = 20;

  f_colour = vec4(uv, 0, 1);
}
