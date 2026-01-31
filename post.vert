#version 450

layout(location = 0) out vec2 f_pos;

void main() {
  vec2 pos = vec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1) * 3;

  gl_Position = vec4(pos * 2.0 - 1.0, 0, 1);
  gl_Position.x *= -1;

  f_pos = pos;
  f_pos.x = 1.0 - f_pos.x;
}
