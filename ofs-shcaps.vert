#version 450
#pragma leco include "ofs.glsl"

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 i_pos;

vec4 light();
vec3 proj(vec4 pos);

void main() {
  float backface = dot(normal, light().xyz);
  proj((pos + vec4(i_pos, 0)) * step(0, backface));
}
