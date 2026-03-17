#version 450

layout(binding = 0) uniform samplerCube txt;

layout(location = 0) in vec3 f_pos;
layout(location = 0) out vec4 o_colour;

void main() {
  vec3 p = normalize(f_pos);
  o_colour = texture(txt, p);
}

