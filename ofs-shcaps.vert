#version 450
#pragma leco include "ofs.glsl"

layout(location = 3) in vec4 pos;
layout(location = 4) in vec3 normal;

float i_backface(vec3 n);
vec4  i_modl(vec4);
vec3  proj(vec4 pos);

void main() {
  proj(i_modl(pos) * i_backface(normal));
}
