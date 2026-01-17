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
  float lsz = texture(shadowmap, f_lspos.xy * 0.5 + 0.5).r;
  float bias = 0.005;
  float shadow = f_lspos.z - bias < lsz ? 1 : 0;

  vec4 c = texture(nonuniformEXT(txts[f_txtid]), f_uv);
  c.rgb *= shadow;

  colour = c;
  position = vec4(f_pos, 1);
  normal = vec4(f_normal, 1);
}
