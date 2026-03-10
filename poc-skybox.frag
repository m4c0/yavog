#version 450

layout(location = 0) in vec2 f_pos;
layout(location = 0) out vec4 o_colour;

void main() {
  o_colour = vec4(1, f_pos, 1);
}

