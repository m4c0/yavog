#version 450
#pragma leco include "buffers.glsl"

layout(location = 3) in vec4 pos;
layout(location = 4) in vec3 normal;
layout(location = 5) in vec2 uv;
layout(location = 2, component = 3) in float i_txtid;

layout(location = 0) out vec2 f_uv;
layout(location = 1) out uint f_txtid;
layout(location = 2) out vec3 f_pos;
layout(location = 3) out vec3 f_normal;

vec3  i_qrot(vec3);
vec4  i_modl(vec4);

const float far = 100.0;
const float near = 0.01;
const float fov = radians(90);

vec3 proj(vec4 p) {
  float f = 1.0 / tan(fov / 2.0);

  // TODO: adjust to camera
  p.xy *= -1; // Left-hand to right-hand

  vec3 ret = p.xyz;

  gl_Position = mat4(
    f, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -(far * near) / (far - near), 0
  ) * p;

  return ret;
}

void main() {
  f_pos = proj(i_modl(pos));
  f_uv = uv;
  f_txtid = int(i_txtid);
  f_normal = i_qrot(normal);
}
