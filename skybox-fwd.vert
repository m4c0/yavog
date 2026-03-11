#version 450

layout(push_constant) uniform upc {
  float aspect;
} pc;

layout(location = 0) out vec3 f_pos;

void main() {
  vec2 pos = vec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1) * 3;

  vec3 p3 = vec3(pos * 2 - 1, 1);

  gl_Position = vec4(p3, 1);

  p3.x *= pc.aspect;
  f_pos = p3;
}
