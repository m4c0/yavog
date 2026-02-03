export module prism;
import clay;
import dotz;
import models;
import no;
import ofs;

constexpr dotz::vec4 pos[] {
  {  0.5, -0.5,  0.5, 1.0 }, // 0
  {  0.5, -0.5, -0.5, 1.0 }, // 1
  { -0.5,  0.5,  0.5, 1.0 }, // 2
  { -0.5,  0.5, -0.5, 1.0 }, // 3
  { -0.5, -0.5,  0.5, 1.0 }, // 4
  { -0.5, -0.5, -0.5, 1.0 }, // 5
};
constexpr const auto s = 0.7071067812; // sqrt(2) / 2
constexpr const models::vtx vtx[] {
  { .id = 5, .uv { 1, 0 }, .normal {  0,  0, -1 } },
  { .id = 3, .uv { 1, 1 }, .normal {  0,  0, -1 } },
  { .id = 1, .uv { 0, 0 }, .normal {  0,  0, -1 } },

  { .id = 0, .uv { 1, 0 }, .normal {  0,  0,  1 } },
  { .id = 2, .uv { 0, 1 }, .normal {  0,  0,  1 } },
  { .id = 4, .uv { 0, 0 }, .normal {  0,  0,  1 } },

  { .id = 2, .uv { 0, 1 }, .normal { -1,  0,  0 } },
  { .id = 3, .uv { 1, 1 }, .normal { -1,  0,  0 } },
  { .id = 4, .uv { 0, 0 }, .normal { -1,  0,  0 } },
  { .id = 5, .uv { 1, 0 }, .normal { -1,  0,  0 } },

  { .id = 0, .uv { 1, 0 }, .normal {  0, -1,  0 } },
  { .id = 1, .uv { 1, 1 }, .normal {  0, -1,  0 } },
  { .id = 4, .uv { 0, 0 }, .normal {  0, -1,  0 } },
  { .id = 5, .uv { 0, 1 }, .normal {  0, -1,  0 } },

  { .id = 0, .uv { 1, 0 }, .normal {  s,  s,  0 } },
  { .id = 1, .uv { 1, 1 }, .normal {  s,  s,  0 } },
  { .id = 2, .uv { 0, 0 }, .normal {  s,  s,  0 } },
  { .id = 3, .uv { 0, 1 }, .normal {  s,  s,  0 } },
};
constexpr const models::tri tri[] {
  { 0, 1, 2 },
  { 3, 4, 5 },

  {  6,  7,  8 }, {  9,  8,  7 },
  { 12, 11, 10 }, { 11, 12, 13 },
  { 14, 15, 16 }, { 17, 16, 15 },
};
constexpr const models::edge edg[] {
  { 5, 3 }, { 3, 1 }, { 1, 5 },
  { 0, 2 }, { 2, 4 }, { 4, 0 },
  { 2, 3 }, { 5, 4 }, { 0, 1 },
};

export namespace prism {
  struct t {
    static constexpr const auto & pos = ::pos;
    static constexpr const auto & vtx = ::vtx;
    static constexpr const auto & tri = ::tri;
    static constexpr const auto & edg = ::edg;
  };
}
