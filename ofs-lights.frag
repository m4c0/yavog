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

const float uv_scale = 5;
const float uv_exp = 2;

vec4 txt(vec2 uv) { return texture(nonuniformEXT(txts[f_txtid]), uv); }
vec4 tri_plane_txt() {
  vec3 p = f_pos / uv_scale;
  vec3 w = pow(abs(f_normal), vec3(uv_exp));
  vec4 sum = txt(p.zy) * w.x + txt(p.xz) * w.y + txt(p.xy) * w.z;
  return sum / (w.x + w.y + w.z);
}

void main() {
  vec4 clr = tri_plane_txt();
  float ln = clamp(dot(f_normal, -pc.light.xyz), 0, 1) * 0.8 + 0.2;

  colour = vec4(clr.rgb * ln, clr.a);
}
