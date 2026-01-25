#version 450
#pragma leco include "ofs.glsl"

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 uv;
layout(location = 2, component = 0) in vec3 i_pos;
layout(location = 2, component = 3) in float txtid;

vec3 proj(vec4);

layout(location = 0) out vec2 f_uv;
layout(location = 1) out uint f_txtid;

void main() {
  proj(pos + vec4(i_pos, 0));
  f_uv = uv;
  f_txtid = int(txtid);
}
