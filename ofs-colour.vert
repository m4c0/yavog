#version 450
#pragma leco include "ofs.glsl"

layout(location = 3) in vec4 pos;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec3 normal;
layout(location = 0, component = 3) in float i_txtid;

layout(location = 0) out vec2 f_uv;
layout(location = 1) out uint f_txtid;
layout(location = 2) out vec3 f_pos;
layout(location = 3) out vec3 f_normal;

vec3  i_qrot(vec3);
vec4  i_modl(vec4);
vec3  proj(vec4 pos);

void main() {
  f_pos = proj(i_modl(pos));
  f_uv = uv;
  f_txtid = int(i_txtid);
  f_normal = i_qrot(normal);
}
