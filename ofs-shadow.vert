#version 450

layout(push_constant) uniform upc {
  float aspect;
  float fov_deg;
  float sun_angle;
} pc;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 i_pos;

const float near =  0.01;
const float far  = 10.0;

void main() {
  float f = 1.0 / tan(radians(pc.fov_deg) / 2.0);

  // TODO: adjust to camera
  vec4 p = pos.w == 0
    ? vec4(0, -1, 0, 0) // Point at infinity, oriented to the light
    : vec4(pos.xyz + i_pos, 1);
  p.xy *= -1; // Left-hand to right-hand

  gl_Position = mat4(
    f / pc.aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -(far * near) / (far - near), 0
  ) * p;
}
