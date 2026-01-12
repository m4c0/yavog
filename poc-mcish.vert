#version 450

layout(push_constant) uniform upc {
  float aspect;
  float fov_deg;
} pc;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3, component = 0) in vec3 i_pos;
layout(location = 3, component = 3) in float txtid;

layout(location = 0) out vec2 f_uv;
layout(location = 1) out uint f_txtid;
layout(location = 2) out vec4 f_pos;
layout(location = 3) out vec3 f_normal;

const float near =  0.01;
const float far  = 10.0;

void main() {
  float f = 1.0 / tan(radians(pc.fov_deg) / 2.0);

  vec3 p = pos.xyz + i_pos; // Vertex + Model
  p.xy *= -1; // Left-hand to right-hand
  gl_Position = f_pos = vec4( // Projection
    p.x * f / pc.aspect,
    p.y * f,
    p.z * (p.z - near) / (far - near),
    p.z
  );

  f_normal = normal;
  f_uv = uv;
  f_txtid = int(txtid);
}
