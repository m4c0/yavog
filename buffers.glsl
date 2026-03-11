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
