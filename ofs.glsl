#version 450

layout(push_constant) uniform upc {
  vec4  light;
  float aspect;
  float fov_deg;
  float far;
} pc;

const float near =   0.01;

float backface(vec3 n) {
  return step(0, dot(n, pc.light.xyz));
}

vec3 proj(vec4 pos) {
  float f = 1.0 / tan(radians(pc.fov_deg) / 2.0);

  // TODO: adjust to camera
  vec4 p = pos.w == 0 ? pc.light : vec4(pos.xyz, 1);
  p.xy *= -1; // Left-hand to right-hand

  vec3 ret = p.xyz;

  float far = pc.far;
  gl_Position = mat4(
    f / pc.aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -(far * near) / (far - near), 0
  ) * p;

  return ret;
}
