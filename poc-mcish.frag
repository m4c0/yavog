#version 450

layout(set = 0, binding = 0) uniform sampler2D txts[1];

layout(location = 0) in  vec2 f_uv;
layout(location = 1) in  flat uint f_txtid;
layout(location = 0) out vec4 colour;

void main() {
  colour = texture(txts[f_txtid], f_uv);
}
