#pragma leco add_shader "chunk-stamp.comp"
export module chunk:stamp;
import :cpipeline;
import :objects;
import dotz;
import voo;

namespace chunk {
  export inline constexpr const auto minmax = 15;
  export inline constexpr const auto len = minmax * 2 + 1;
  static inline constexpr const auto blk_size = len * len * len;

  export struct block {
    dotz::vec4 rot { 0, 0, 0, 1 };
    model mdl = model::none;
    unsigned txt;
    unsigned _pad[2];
  };
  
  export class stamp {
    struct upc {
      dotz::ivec3 center;
    };

    voo::bound_buffer m_buf = voo::bound_buffer::create_from_host(
        blk_size * sizeof(block),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    voo::mapmem m_m { *m_buf.memory };
    block * m_data = static_cast<block *>(*m_m);

    cpipeline<upc, 2> m_cp;

    voo::single_cb m_cb {};
    voo::fence m_f { true };

  public:
    explicit stamp(VkBuffer buf, dotz::ivec3 l) :
      m_cp {
        "chunk-stamp.comp.spv",
        vee::specialisation_info<dotz::ivec3>(l, {
          vee::specialisation_map_entry(94, &dotz::ivec3::x),
          vee::specialisation_map_entry(95, &dotz::ivec3::y),
          vee::specialisation_map_entry(96, &dotz::ivec3::z),
        }),
        { *m_buf.buffer, buf }
      }
    {}

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

    auto data() { return m_data; }

    void copy(dotz::ivec3 c) {
      m_f.wait_and_reset();
      voo::run(voo::cmd_buf_one_time_submit { m_cb.cb() }, [&] {
        upc pc { .center = c };
        m_cp.cmd_dispatch(m_cb.cb(), &pc, { 0, 1 }, { len, len, len });
      });
      voo::queue::universal()->queue_submit({
        .fence = m_f,
        .command_buffer = m_cb.cb(),
      });
    }
  };
}
