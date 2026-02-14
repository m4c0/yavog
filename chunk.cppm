export module chunk;
import dotz;
import hai;
import sv;

namespace chunk {
  export inline constexpr const auto minmax = 15;
  export inline constexpr const auto len = minmax * 2 + 1;
  static inline constexpr const auto blk_size = len * len * len;

  export enum class model {
    none,
    corner,
    cube,
    prism,
  };

  export struct block {
    dotz::vec4 rot { 0, 0, 0, 1 };
    model mdl = model::none;
    unsigned txt;
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

    void build(auto & m, model mdl, dotz::vec3 c) const {
      for (auto x = -minmax; x <= minmax; x++) {
        for (auto y = -minmax; y <= minmax; y++) {
          for (auto z = -minmax; z <= minmax; z++) {
            auto p = dotz::ivec3 { x, y, z };
            auto d = at(p);
            if (d.mdl != mdl) continue;
            m += {
              .pos { p + c, d.txt },
              .rot = d.rot,
            };
          }
        }
      }
    }
  };
}
