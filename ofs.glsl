#version 450
#pragma leco include "quaternions.glsl"

layout(push_constant) uniform upc {
  vec4  light;
  float aspect;
  float fov_deg;
  float far;
} pc;

layout(location = 0, component = 0) in vec3 i_pos;
layout(location = 0, component = 3) in float i_txtid;
layout(location = 1) in vec4 i_rot;
layout(location = 2) in vec3 i_size;

const float near =   0.01;

vec3 qrot(vec3 p, vec4 q);

float backface(vec3 n, vec4 rot) {
  return step(0, dot(qrot(n, rot), pc.light.xyz));
}
float i_backface(vec3 n) {
  return backface(n, i_rot);
}
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

vec3 proj(vec4 pos) {
  float f = 1.0 / tan(radians(pc.fov_deg) / 2.0);

  // TODO: adjust to camera
  vec4 p = pos.w == 0 ? pc.light : vec4(pos.xyz, 1);
  p.xy *= -1; // Left-hand to right-hand

  vec3 ret = p.xyz;

  float far = pc.far;
  gl_Position = mat4(
    f / pc.aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -(far * near) / (far - near), 0
  ) * p;

  return ret;
}
