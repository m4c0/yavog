#version 450
#pragma leco include "buffers.glsl"

layout(location = 3) in vec4 pos;
layout(location = 4) in vec3 normal;
layout(location = 5) in vec2 uv;
layout(location = 2, component = 3) in float i_txtid;

layout(location = 0) out vec2 f_uv;
layout(location = 1) out uint f_txtid;
layout(location = 2) out vec3 f_pos;
layout(location = 3) out vec3 f_normal;

vec3  i_qrot(vec3);
vec4  i_modl(vec4);

void main() {
  vec4 p = i_modl(pos);
  f_pos = p.xyz;
  f_uv = uv;
  f_txtid = int(i_txtid);
  f_normal = i_qrot(normal);

  const float far = 5;
  vec3 q = normalize(p.xyz);

  const float pi = 3.14159265358979323;
  float lng = atan(p.z, p.x);
  float lat = asin(q.y);
  vec2 uv = vec2(lng / pi, lat / (pi / 2));
  gl_Position = vec4(uv, length(p) / far, p.w);
}
