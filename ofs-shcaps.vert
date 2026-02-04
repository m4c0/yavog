#version 450
#pragma leco include "ofs.glsl"

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 i_pos;
layout(location = 3) in vec4 i_rot;

float backface(vec3 n, vec4 rot);
vec4 modl(vec4, vec3, vec4);
vec3 proj(vec4 pos);

void main() {
  proj(modl(pos, i_pos, i_rot) * backface(normal, i_rot));
}
