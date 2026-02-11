export module chunk;
import dotz;
import embedded;
import hai;
import sv;

namespace chunk {
  export inline constexpr const auto minmax = 15;
  export inline constexpr const auto len = minmax * 2 + 1;
  static inline constexpr const auto blk_size = len * len * len;

  export enum class model {
    corner,
    cube,
    prism,
  };

  export struct block {
    dotz::vec4 rot { 0, 0, 0, 1 };
    model mdl = model::cube;
    sv txt;
  };
  
  export class t {
    hai::array<block> m_data { blk_size };

  public:
    [[nodiscard]] auto & at(dotz::ivec3 p) {
      p = p + minmax;
      if (p.x < 0 || p.y < 0 || p.z < 0) throw 0;
      if (p.x >= len || p.y >= len || p.z >= len) throw 0;
      return m_data[p.x + p.y * len + p.z * len * len];
    }

    void build(embedded::builder & m, model mdl, dotz::vec3 c) const {
      for (auto x = 0; x < len; x++) {
        for (auto y = 0; y < len; y++) {
          for (auto z = 0; z < len; z++) {
            auto p = dotz::vec3 { x, y, z } - minmax;
            m += {
              .pos { p - c + 0.5, 1 },
            };
          }
        }
      }
    }
  };
}
