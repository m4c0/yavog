#extension GL_EXT_multiview : require
#pragma leco include "buffers.glsl"

layout(push_constant) uniform upc {
  float far;
  float near;
} pc;

layout(location = 3) in vec4 pos;
layout(location = 4) in vec3 normal;
layout(location = 5) in vec2 uv;
layout(location = 2, component = 3) in float i_txtid;

layout(location = 0) out vec2 f_uv;
layout(location = 1) out uint f_txtid;
layout(location = 2) out vec3 f_pos;
layout(location = 3) out vec3 f_normal;

vec3  qrot(vec3 p, vec4 q);
vec3  i_qrot(vec3);
vec4  i_modl(vec4);

const float fov = 90;

struct proj_params {
  float fov_deg;
  float aspect;
  float far;
  float near;
};
vec3 proj(vec4 p, proj_params par);

vec3 axis_rot(vec3 p, vec3 axis, float deg) {
  float ang = radians(deg / 2);
  return qrot(p, vec4(axis * sin(ang), cos(ang)));
}

vec3 view(vec3 p) {
  // Projection is inverted compared to player's camera
  switch (gl_ViewIndex) {
    // neg x
    case 0: return axis_rot(p, vec3(0, 1, 0), 90);
    // pos x
    case 1: return axis_rot(p, vec3(0, 1, 0), -90);
    // pos y
    case 2: return axis_rot(p, vec3(1, 0, 0), 90);
    // neg y
    case 3: return axis_rot(p, vec3(1, 0, 0), -90);
    // neg z
    case 4: return p;
    // pos z
    case 5: return axis_rot(p, vec3(0, 1, 0), 180);
  }
}
vec4 view(vec4 p) {
  return vec4(view(p.xyz), p.w);
}

void main() {
  proj_params par;
  par.fov_deg = fov;
  par.aspect = 1;
  par.far = pc.far;
  par.near = pc.near;
  f_pos = proj(view(i_modl(pos)), par);
  f_uv = uv;
  f_txtid = int(i_txtid);
  f_normal = view(i_qrot(normal));
}
