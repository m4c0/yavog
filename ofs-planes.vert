#version 450
#pragma leco include "ofs.glsl"

vec3 proj(vec4);

void main() {
  vec4 pos = vec4(gl_VertexIndex & 1, -2.0, (gl_VertexIndex >> 1) & 1, 1);
  pos.xz *= 200.0;
  pos.xz -= 100.0;

  proj(pos);
}
