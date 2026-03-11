#version 450

layout(binding = 0) uniform sampler2D txt;

layout(location = 0) in vec3 f_pos;
layout(location = 0) out vec4 o_colour;

void main() {
  vec3 p = normalize(f_pos);

  const float pi = 3.14159265358979323;
  float lng = atan(p.z, p.x);
  float lat = asin(p.y);

  vec2 uv = vec2(lng / pi, lat / (pi / 2));
  uv = uv * 0.5 + 0.5;
  o_colour = texture(txt, uv);
}

