#version 450
#pragma leco include "quaternions.glsl"

layout(location = 0, component = 0) in vec3 i_pos;
layout(location = 1) in vec4 i_rot;
layout(location = 2) in vec3 i_size;
layout(location = 2, component = 3) in float i_txtid;

vec3 qrot(vec3 p, vec4 q);

vec3 i_qrot(vec3 p) {
  return qrot(p, i_rot);
}

vec4 modl(vec4 pos, vec3 i_pos, vec4 i_rot) {
  // TODO: apply rotation to normals
  return vec4(qrot(pos.xyz, i_rot), pos.w) + vec4(i_pos, 0);
}
vec4 i_modl(vec4 pos) {
  return modl(pos * vec4(i_size, 1), i_pos, i_rot);
}

struct proj_params {
  float fov_deg;
  float aspect;
  float far;
  float near;
};
vec3 proj(vec4 p, proj_params par) {
  float f = 1.0 / tan(radians(par.fov_deg) / 2.0);

  // TODO: adjust to camera
  p.xy *= -1; // Left-hand to right-hand

  vec3 ret = p.xyz;

  gl_Position = mat4(
    f / par.aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, par.far / (par.far - par.near), 1,
    0, 0, -(par.far * par.near) / (par.far - par.near), 0
  ) * p;

  return ret;
}
