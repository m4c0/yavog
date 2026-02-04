#version 450
#pragma leco include "ofs.glsl"

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3, component = 0) in vec3 i_pos;
layout(location = 3, component = 3) in float txtid;
layout(location = 4) in vec4 i_rot;

layout(location = 0) out vec2 f_uv;
layout(location = 1) out uint f_txtid;
layout(location = 2) out vec3 f_normal;

vec3 proj(vec4);

void main() {
  proj(pos + vec4(i_pos, 0));
  f_uv = uv;
  f_txtid = int(txtid);
  f_normal = normal;
}
