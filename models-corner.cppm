export module models:corner;
import :common;
import dotz;

static constexpr dotz::vec4 pos[] {
  {  0.5, -0.5, -0.5, 1.0 }, // 0
  { -0.5,  0.5, -0.5, 1.0 }, // 1
  { -0.5, -0.5,  0.5, 1.0 }, // 2
  { -0.5, -0.5, -0.5, 1.0 }, // 3
};
static constexpr const auto s = 0.5773502692; // sqrt(3) / 3
static constexpr const models::vtx vtx[] {
  { .id = 3, .uv { 0, 1 }, .normal {  0,  0, -1 } },
  { .id = 1, .uv { 1, 1 }, .normal {  0,  0, -1 } },
  { .id = 0, .uv { 1, 0 }, .normal {  0,  0, -1 } },

  { .id = 3, .uv { 0, 1 }, .normal { -1,  0,  0 } },
  { .id = 2, .uv { 1, 0 }, .normal { -1,  0,  0 } },
  { .id = 1, .uv { 1, 1 }, .normal { -1,  0,  0 } },

  { .id = 3, .uv { 1, 1 }, .normal {  0, -1,  0 } },
  { .id = 0, .uv { 1, 0 }, .normal {  0, -1,  0 } },
  { .id = 2, .uv { 0, 1 }, .normal {  0, -1,  0 } },

  { .id = 0, .uv { 1, 0 }, .normal {  s,  s,  s } },
  { .id = 1, .uv { 1, 1 }, .normal {  s,  s,  s } },
  { .id = 2, .uv { 0, 1 }, .normal {  s,  s,  s } },
};
static constexpr const models::tri tri[] {
  { 0,  1,  2 },
  { 3,  4,  5 },
  { 6,  7,  8 },
  { 9, 10, 11 },
};
static constexpr const models::edge edg[] {
  { 0, 1 }, { 0, 2 }, { 0, 3 },
  { 1, 2 }, { 1, 3 },
  { 2, 3 },
};

export namespace models::corner {
  struct t {
    static constexpr const auto id = 'crnr';

    static constexpr const auto & pos = ::pos;
    static constexpr const auto & vtx = ::vtx;
    static constexpr const auto & tri = ::tri;
    static constexpr const auto & edg = ::edg;
  };
}

