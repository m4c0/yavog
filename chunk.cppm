export module chunk;
import dotz;
import embedded;
import hai;
import sv;

namespace chunk {
  static inline constexpr const auto len = 32;
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
      return m_data[p.x + p.y * len + p.z * len * len];
    }

    void build(embedded::builder & m, model mdl, dotz::vec3 c) const {
      for (auto x = 0; x < len; x++) {
        for (auto y = 0; y < len; y++) {
          for (auto z = 0; z < len; z++) {
            dotz::vec3 p { x, y, z };
            m += {
              .pos { p - c + 0.5, 1 },
            };
          }
        }
      }
    }
  };
}
