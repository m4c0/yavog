#version 450

layout(set = 0, binding = 0) uniform sampler2D u_colour;
layout(set = 0, binding = 1) uniform sampler2D u_depth;

layout(location = 0) in vec2 f_pos;
layout(location = 0) out vec4 o_colour;

void main() {
  float depth = texture(u_depth, f_pos).r;
  vec4 colour = texture(u_colour, f_pos);
  o_colour = vec4(colour.rgb * (1.0 - depth), colour.a);
}
