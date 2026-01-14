#version 450

#define PI 3.14159265358979323
#define SMP_SZ 4
#define GAUSS_STDDEV 2
#define GAUSS_STDVAR (GAUSS_STDDEV * GAUSS_STDDEV)

const vec2 scr_sz = vec2(800, 600);

layout(set = 0, binding = 0) uniform sampler2D u_colour;
layout(set = 0, binding = 1) uniform sampler2D u_position;
layout(set = 0, binding = 2) uniform sampler2D u_normal;
layout(set = 0, binding = 3) uniform sampler2D u_depth;

layout(location = 0) in vec2 f_pos;
layout(location = 0) out vec4 o_colour;

void main() {
  // vec4 position = texture(u_position, f_pos);
  // vec4 normal = texture(u_normal, f_pos);
  // float depth = texture(u_depth, f_pos).r;
  // vec4 colour = texture(u_colour, f_pos);

  // https://dl.acm.org/doi/epdf/10.1145/1141911.1142016
  // https://en.wikipedia.org/wiki/Convolution#Discrete_convolution
  // https://en.wikipedia.org/wiki/Gaussian_filter#Definition
  vec3 fgn = vec3(0);
  for (int my = -SMP_SZ; my < SMP_SZ; my++) {
    for (int mx = -SMP_SZ; mx < SMP_SZ; mx++) {
      vec2 m = vec2(mx, my) / scr_sz;
      vec3 fnm = texture(u_colour, f_pos - m).rgb;

      float e_exp = -(mx * mx + my * my) / (2 * GAUSS_STDVAR);
      float gm = exp(e_exp) / (2 * PI * GAUSS_STDVAR);

      fgn += fnm * gm;
    }
  }

  vec3 d = texture(u_colour, f_pos).rgb;
  vec3 dd = fgn - d;

  o_colour = vec4(dd, 1);
}
