#version 450
#pragma leco include "ofs.glsl"

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 i_pos;

vec3 proj(vec4 pos);

void main() {
  proj(pos + vec4(i_pos, 0));
}
