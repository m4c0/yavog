#version 450
#pragma leco include "ofs.glsl"

layout(location = 0, component = 0) in vec3 nrm_a;
layout(location = 0, component = 3) in float idx;
layout(location = 1) in vec4 nrm_b;
layout(location = 2) in vec4 vtx_a;
layout(location = 3) in vec4 vtx_b;
layout(location = 4) in vec3 i_pos;
layout(location = 5) in vec4 i_rot;

float backface(vec3 n, vec4 rot);
vec4 modl(vec4, vec3, vec4);
vec3 proj(vec4 pos);

void main() {
  float back_a = backface(nrm_a.xyz, i_rot);
  float back_b = backface(nrm_b.xyz, i_rot);

  if (idx == 0) proj(vec4(0));
  else if (back_a == back_b) gl_Position = vec4(0);
  else if (back_a == 0) {
    proj(modl(mix(vtx_a, vtx_b, idx - 1), i_pos, i_rot));
  } else {
    proj(modl(mix(vtx_b, vtx_a, idx - 1), i_pos, i_rot));
  }
}
