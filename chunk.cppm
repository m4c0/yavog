export module chunk;
import dotz;
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
  };
}
