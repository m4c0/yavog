#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D txts[128];

layout(location = 0) in vec2 f_uv;
layout(location = 1) in flat uint f_txtid;
layout(location = 2) in vec3 f_pos;
layout(location = 3) in vec3 f_normal;

layout(location = 0) out vec4 colour;
layout(location = 1) out vec4 position;
layout(location = 2) out vec4 normal;

const vec3 amb = vec3(0.2);

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
  colour = tri_plane_txt() * vec4(amb, 1);
  position = vec4(f_pos, 1);
  normal = vec4(f_normal, 1);
}
