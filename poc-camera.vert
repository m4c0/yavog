#pragma leco include "quaternions.glsl"

layout(push_constant) uniform upc {
  vec3 cam_pos;
  vec2 cam_rot;
} pc;

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

vec3 axis_rot(vec3 p, vec3 axis, float deg);

void main() {
  vec4 p = (pos * i_size) + i_pos - vec4(pc.cam_pos, 0);
  p.xyz = axis_rot(p.xyz, vec3(0, 1, 0), pc.cam_rot.y);
  p.xyz = axis_rot(p.xyz, vec3(1, 0, 0), pc.cam_rot.x);

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
