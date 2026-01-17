#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3, component = 0) in vec3 i_pos;
layout(location = 3, component = 3) in float txtid;

const float range = 20.0f; // About 2*far plane

void main() {
  vec3 p = pos.xyz + i_pos; // Vertex + Model
  p.xy *= -1; // Left-hand to right-hand

  float a = radians(30);
  const mat3 rot = mat3(
    cos(a), 0, sin(a),
    0, 1, 0,
    -sin(a), 0, cos(a)
  );

  // TODO: adjust to camera
  p = vec3(p.x, -p.z, p.y);
  p = rot * p;

  gl_Position = vec4( // Projection
    p.x / range,
    p.y / range,
    (p.z / range) * 0.5 + 0.5,
    1);
}
