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

// Hamilton Product
vec4 ham(vec4 q1, vec4 q2) {
  vec4 qr;
  qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
  qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
  qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
  qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
  return qr;
}
vec4 modl(vec4 pos, vec3 i_pos, vec4 i_rot) {
  // TODO: apply xy * -1
  // TODO: apply rotation to normals
  // H(R', H(R, P))
  vec4 p = ham(ham(i_rot, vec4(pos.xyz, 0)), vec4(-i_rot.xyz, i_rot.w));
  return vec4(p.xyz, pos.w) + vec4(i_pos, 0);
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
