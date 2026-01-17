#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3, component = 0) in vec3 i_pos;
layout(location = 3, component = 3) in float txtid;

const float near =  0.01;
const float far  = 10.0;

void main() {
  vec3 p = pos.xyz + i_pos; // Vertex + Model
  p.xy *= -1; // Left-hand to right-hand

  float a = radians(45);
  float b = radians(90);
  mat3 rot = mat3(
    1, 0, 0,
    0, cos(b), -sin(b),
    0, sin(b), cos(b)
  ) * mat3(
    cos(a), 0, -sin(a),
    0, 1, 0,
    sin(a), 0, cos(a)
  );
  p = rot * p;

  gl_Position = vec4( // Projection
    p.x,
    p.y,
    far * (p.z - near) / (p.z * (far - near)),
    1);
}
