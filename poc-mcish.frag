#version 450

layout(location = 0) in  vec2 f_uv;
layout(location = 0) out vec4 colour;

void main() {
  vec2 p = f_uv * vec2(2, 4);
  p.x += floor(p.y) * 0.25;
  p = fract(p);
  p = abs(p - 0.5) - vec2(0.5);

  float d = length(max(p, 0.0)) + min(max(p.x, p.y), 0.0);
  d = 1.0 - smoothstep(-0.3, 0, d);

  colour = vec4(d, 0, 0, 1);
}
