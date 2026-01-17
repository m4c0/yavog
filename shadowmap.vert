#version 450
#pragma leco include "shadowmap.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3, component = 0) in vec3 i_pos;
layout(location = 3, component = 3) in float txtid;

vec3 to_light_space(vec3 p);

void main() {
  vec3 p = pos.xyz + i_pos; // Vertex + Model
  p.xy *= -1; // Left-hand to right-hand

  gl_Position = vec4(to_light_space(p), 1);
}
