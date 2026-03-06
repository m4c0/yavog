export module chunk;
export import :bitonic;
export import :compact;
export import :count;
export import :gpunator;
export import :objects;
export import :stamp;
import dotz;
import hai;
import sv;
import voo;

namespace chunk {
  static inline constexpr const auto blk_size = len * len * len;
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

    void copy(auto & m, dotz::ivec3 c, unsigned len) const {
      for (auto z = -minmax; z <= minmax; z++) {
        auto pz = c.z + z + minmax;
        if (pz < 0 || pz >= len) continue;
        for (auto y = -minmax; y <= minmax; y++) {
          auto py = c.y + y + minmax;
          if (py < 0 || py >= len) continue;
          for (auto x = -minmax; x <= minmax; x++) {
            auto px = c.x + x + minmax;
            if (px < 0 || px >= len) continue;

            dotz::ivec3 p { x, y, z };
            auto i = at(p);

            auto pp = pz * len * len + py * len + px;
            m[pp] = {
              .rot = i.rot,
              .pos = dotz::vec3 { p } + c,
              .mdl = static_cast<float>(i.mdl),
              .size { 1 },
              .txtid = static_cast<float>(i.txt),
            };
          }
        }
      }
    }
  };
}
