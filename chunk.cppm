export module chunk;
import dotz;
import hai;
import sv;

namespace chunk {
  export inline constexpr const auto minmax = 15;
  export inline constexpr const auto len = minmax * 2 + 1;
  static inline constexpr const auto blk_size = len * len * len;

  export enum class model : unsigned {
    none = 0,
    corner,
    cube,
    prism,
  };

  export struct block {
    dotz::vec4 rot { 0, 0, 0, 1 };
    model mdl = model::none;
    unsigned txt;
  };
  struct inst : block {
    dotz::vec3 pos {};
    dotz::vec3 size { 1 };
    bool set = false;
  };
  
  export class t {
    hai::array<block> m_data { blk_size };

  public:
    void set(dotz::ivec3 p, block b) {
      p = p + minmax;
      if (p.x < 0 || p.y < 0 || p.z < 0) throw 0;
      if (p.x >= len || p.y >= len || p.z >= len) throw 0;
      m_data[p.x + p.y * len + p.z * len * len] = b;
    }
    [[nodiscard]] auto & at(dotz::ivec3 p) const {
      p = p + minmax;
      if (p.x < 0 || p.y < 0 || p.z < 0) throw 0;
      if (p.x >= len || p.y >= len || p.z >= len) throw 0;
      return m_data[p.x + p.y * len + p.z * len * len];
    }

    void copy(auto & m, dotz::vec3 c, float mult = 1) const {
      for (auto x = -minmax; x <= minmax; x++) {
        for (auto y = -minmax; y <= minmax; y++) {
          for (auto z = -minmax; z <= minmax; z++) {
            dotz::ivec3 p { x, y, z };
            auto i = at(p);
            m += {
              .rot = i.rot,
              .pos = dotz::vec3 { p } * mult + c,
              .mdl = static_cast<float>(i.mdl),
              .size { 1 },
              .txt = static_cast<float>(i.txt),
            };
          }
        }
      }
    }

    void build(auto & m, model mdl, dotz::vec3 c, float mult = 1) const {
      for (auto x = -minmax; x <= minmax; x++) {
        for (auto y = -minmax; y <= minmax; y++) {
          inst i {};
          const auto stamp = [&](int dd) {
            if (!i.set) return;
            m += {
              .rot = i.rot,
              .pos { dotz::vec3 { i.pos } * mult + c, i.txt },
              .size = i.size,
            };
            i = {};
          };

          for (auto z = -minmax; z <= minmax; z++) {
            auto p = dotz::ivec3 { x, y, z };
            inst d { at(p), p };
            if (d.mdl != mdl) {
              stamp(0);
              continue;
            }
            if (!i.set) {
              i = d;
              i.set = true;
              continue;
            }
            if (dotz::sq_length(i.rot - d.rot) < 0.0001 && i.txt == d.txt && i.mdl == d.mdl) {
              i.pos.z += 0.5;
              i.size.z += 1.0;
              continue;
            }
            stamp(1);
          }
          stamp(2);
        }
      }
    }
  };
}
