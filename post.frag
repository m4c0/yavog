#version 450

#define PI 3.14159265358979323
#define SMP_SZ 8
#define GAUSS_STDDEV 3
#define GAUSS_STDVAR (GAUSS_STDDEV * GAUSS_STDDEV)

layout(push_constant) uniform upc {
  vec2 scr_sz;
};

layout(set = 0, binding = 0) uniform sampler2D u_colour;
layout(set = 0, binding = 1) uniform sampler2D u_position;
layout(set = 0, binding = 2) uniform sampler2D u_normal;
layout(set = 0, binding = 3) uniform sampler2D u_depth;

layout(location = 0) in vec2 f_pos;
layout(location = 0) out vec4 o_colour;

#define DEPTH_READ(p) (texture(u_position, (p)).z / 10.0)
vec3 unsharp_mask_depth_buffer(vec3 c) {
  // https://dl.acm.org/doi/epdf/10.1145/1141911.1142016
  // https://en.wikipedia.org/wiki/Convolution#Discrete_convolution
  // https://en.wikipedia.org/wiki/Gaussian_filter#Definition
  float fgn = float(0);
  for (int my = -SMP_SZ; my < SMP_SZ; my++) {
    for (int mx = -SMP_SZ; mx < SMP_SZ; mx++) {
      vec2 m = vec2(mx, my) / scr_sz;
      float fnm = DEPTH_READ(f_pos - m);

      float e_exp = -(mx * mx + my * my) / (2 * GAUSS_STDVAR);
      float gm = exp(e_exp) / (2 * PI * GAUSS_STDVAR);

      fgn += fnm * gm;
    }
  }

  float d = DEPTH_READ(f_pos);
  float dd = fgn - d;
  
  return c + dd;
}

void main() {
  vec4 colour = texture(u_colour, f_pos);
  colour.rgb = unsharp_mask_depth_buffer(colour.rgb);
  o_colour = colour;
}
