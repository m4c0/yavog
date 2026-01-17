#version 450

const float range = 20.0f; // About 2*far plane
const float z_range = 10.0f;

vec3 to_light_space(vec3 p, float angle) {
  float a = radians(angle);
  const mat3 rot = mat3(
    cos(a), 0, sin(a),
    0, 1, 0,
    -sin(a), 0, cos(a)
  );

  // TODO: adjust to camera
  p = vec3(p.x, p.z, -p.y);
  p = mat3(
    cos(0.3), sin(0.3), 0,
    -sin(0.3), cos(0.3), 0,
    0, 0, 1
  ) * p;
  p = rot * p;

  return vec3( // Projection
    p.x / range,
    p.y / range,
    (p.z / z_range) * 0.5 + 0.5);
}
