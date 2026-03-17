#version 450
#pragma leco include "buffers.glsl"

layout(push_constant) uniform upc {
  vec4  light;
  float aspect;
  float fov_deg;
  float far;
} pc;

layout(location = 1) in vec4 i_rot;

const float near =   0.01;

vec3 qrot(vec3 p, vec4 q);
vec3 i_qrot(vec3 p);

float backface(vec3 n, vec4 rot) {
  return step(0, dot(qrot(n, rot), pc.light.xyz));
}
float i_backface(vec3 n) {
  return backface(n, i_rot);
}

struct proj_params {
  float fov_deg;
  float aspect;
  float far;
  float near;
};
vec3 proj(vec4 p, proj_params par);
vec3 proj(vec4 pos) {
  proj_params par;
  par.fov_deg = pc.fov_deg;
  par.aspect = pc.aspect;
  par.far = pc.far;
  par.near = near;

  vec4 p = pos.w == 0 ? pc.light : vec4(pos.xyz, 1);
  return proj(p, par);
}
