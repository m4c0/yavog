#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D txts[128];
layout(set = 1, binding = 0) uniform sampler2D shadowmap;

layout(location = 0) in  vec2 f_uv;
layout(location = 1) in  flat uint f_txtid;
layout(location = 2) in  vec3 f_pos;
layout(location = 3) in  vec3 f_normal;
layout(location = 4) in  vec3 f_lspos;

layout(location = 0) out vec4 colour;
layout(location = 1) out vec4 position;
layout(location = 2) out vec4 normal;

void main() {
  vec3 lspos = f_lspos * 0.5 + 0.5;
  float lsz = texture(shadowmap, lspos.xy).r;
  float shadow = step(lsz, f_pos.z);

  colour = shadow * texture(nonuniformEXT(txts[f_txtid]), f_uv);
  position = vec4(f_pos, 1);
  normal = vec4(f_normal, 1);
}
