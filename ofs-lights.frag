#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(push_constant) uniform upc {
  vec4  light;
  float aspect;
  float fov_deg;
} pc;

layout(set = 0, binding = 0) uniform sampler2D txts[128];

layout(location = 0) in vec2 f_uv;
layout(location = 1) in flat uint f_txtid;
layout(location = 2) in vec3 f_pos;
layout(location = 3) in vec3 f_normal;

layout(location = 0) out vec4 colour;

void main() {
  float ln = clamp(dot(f_normal, -pc.light.xyz), 0, 1) * 0.8 + 0.2;
  vec4 txt = texture(nonuniformEXT(txts[f_txtid]), f_uv);

  colour = vec4(txt.rgb * ln, txt.a);
}
