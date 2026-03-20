#version 450

// Hamilton Product
vec4 ham(vec4 q1, vec4 q2) {
  float a1 = q1.w; float b1 = q1.x; float c1 = q1.y; float d1 = q1.z;
  float a2 = q2.w; float b2 = q2.x; float c2 = q2.y; float d2 = q2.z;

  vec4 qr;
  qr.w = a1 * a2 - b1 * b2 - c1 * c2 - d1 * d2; 
  qr.x = a1 * b2 + b1 * a2 + c1 * d2 - d1 * c2;
  qr.y = a1 * c2 - b1 * d2 + c1 * a2 + d1 * b2;
  qr.z = a1 * d2 + b1 * c2 - c1 * b2 + d1 * a2;
  return qr;
}
vec3 qrot(vec3 p, vec4 q) {
  // H(R', H(R, P))
  return ham(ham(q, vec4(p, 0)), vec4(-q.xyz, q.w)).xyz;
}
vec4 qmult(vec4 a, vec4 b) {
  float v0 = a.w; vec3 v = a.xyz;
  float w0 = b.w; vec3 w = b.xyz;
  return vec4(v0 * w + w0 * v + cross(v, w), v0 * w0 - dot(v, w));
}

vec3 axis_rot(vec3 p, vec3 axis, float deg) {
  float ang = radians(deg / 2);
  return qrot(p, vec4(axis * sin(ang), cos(ang)));
}
