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
const float fov = 90;

struct proj_params {
  float fov_deg;
  float aspect;
  float far;
  float near;
};
vec3 proj(vec4 p, proj_params par);

void main() {
  proj_params par;
  par.fov_deg = 90;
  par.aspect = 1;
  par.far = far;
  par.near = near;
  f_pos = proj(i_modl(pos), par);
  f_uv = uv;
  f_txtid = int(i_txtid);
  f_normal = i_qrot(normal);
}
