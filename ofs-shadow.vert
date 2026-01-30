#version 450
#pragma leco include "ofs.glsl"

layout(location = 0) in vec4 nrm_a;
layout(location = 1) in vec4 nrm_b;
layout(location = 2) in vec4 vtx_a;
layout(location = 3) in vec4 vtx_b;
layout(location = 4) in vec3 i_pos;

float backface(vec3 n);
vec3 proj(vec4 pos);

void main() {
  float back_a = backface(nrm_a.xyz);
  float back_b = backface(nrm_b.xyz);

  int idx = gl_VertexIndex % 3;
  if (idx == 2) proj(vec4(0));
  else if (back_a == back_b) proj(vec4(0));
  else if (back_a == 0) {
    proj(mix(vtx_a, vtx_b, idx) + vec4(i_pos, 0));
  } else {
    proj(mix(vtx_b, vtx_a, idx) + vec4(i_pos, 0));
  }
}
