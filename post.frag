#version 450

layout(constant_id = 33) const uint enabled = 1;

layout(push_constant) uniform upc {
  vec2 scr_sz;
};

layout(set = 0, binding = 0) uniform sampler2D u_colour;
layout(set = 0, binding = 1) uniform sampler2D u_position;
layout(set = 0, binding = 2) uniform sampler2D u_normal;

layout(location = 0) in vec2 f_pos;
layout(location = 0) out vec4 o_colour;

const vec3 fog_colour = vec3(0.7, 0.75, 0.8);

#define DEPTH_READ(p) texture(u_position, (p)).z
vec3 unsharp_mask_depth_buffer(vec3 c, float depth) {
  // https://dl.acm.org/doi/epdf/10.1145/1141911.1142016
  // https://en.wikipedia.org/wiki/Convolution#Discrete_convolution
  // https://en.wikipedia.org/wiki/Gaussian_filter#Definition
  // https://en.wikipedia.org/wiki/Kernel_(image_processing)

  float d = depth;
  float dist = 1 + 2000 * pow(smoothstep(0.01, 10, d), 1);

  float fgn = 0;
  for (int my = -2; my <= 2; my++) {
    for (int mx = -2; mx <= 2; mx++) {
      vec2 m = vec2(mx, my) / dist;
      float fnm = DEPTH_READ(f_pos - m);

      float gm = mat3(
        36, 24, 6,
        24, 16, 4,
        6, 4, 1
      )[abs(my)][abs(mx)];
      
      fgn += fnm * gm;
    }
  }
  fgn /= 256;

  float dd = fgn - d;
  
  return (1 - dd * -2) * c;
}

// https://en.wikipedia.org/wiki/Sobel_operator
vec3 sobel(vec3 c) {
  float dist = 10000;

  float gx_n = length(
    -1 * texture(u_normal, f_pos + vec2(-1, -1) / dist).xyz +
    -2 * texture(u_normal, f_pos + vec2(-1,  0) / dist).xyz +
    -1 * texture(u_normal, f_pos + vec2(-1, +1) / dist).xyz +
    +1 * texture(u_normal, f_pos + vec2(+1, -1) / dist).xyz +
    +2 * texture(u_normal, f_pos + vec2(+1,  0) / dist).xyz +
    +1 * texture(u_normal, f_pos + vec2(+1, +1) / dist).xyz
  );

  float gy_n = length(
    -1 * texture(u_normal, f_pos + vec2(-1, -1) / dist).xyz +
    -2 * texture(u_normal, f_pos + vec2( 0, -1) / dist).xyz +
    -1 * texture(u_normal, f_pos + vec2(+1, -1) / dist).xyz +
    +1 * texture(u_normal, f_pos + vec2(-1, +1) / dist).xyz +
    +2 * texture(u_normal, f_pos + vec2( 0, +1) / dist).xyz +
    +1 * texture(u_normal, f_pos + vec2(+1, +1) / dist).xyz
  );

  float g = sqrt(gx_n * gx_n + gy_n * gy_n);
  g = 1 - g * 0.7;
    
  return g * c;
}

vec3 fog(vec3 c, float depth) {
  float f = smoothstep(0.0, 9.0, depth);
  return mix(c, fog_colour, f);
}

void main() {
  float depth = DEPTH_READ(f_pos);

  vec4 colour = texture(u_colour, f_pos);
  if (enabled != 0) {
    colour.rgb = unsharp_mask_depth_buffer(colour.rgb, depth);
    colour.rgb = sobel(colour.rgb);
    colour.rgb = fog(colour.rgb, depth);
  }
  o_colour = colour;
}
