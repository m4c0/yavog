#pragma leco add_shader "chunk-collision.comp"
export module chunk:collision;
import :cpipeline;
import buffers;

namespace chunk {
  static inline constexpr const auto max_ents = 8;

  export struct centity {
    dotz::vec3 pos;
    float hit;
  };
  export class collision {
    struct upc {
      unsigned x;
    };

    voo::bound_buffer m_buf = voo::bound_buffer::create_from_host(
        max_ents * sizeof(centity),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    unsigned m_ecnt = 0;

    cpipeline<upc, 2> m_cp;

    VkBuffer m_dcmd;

  public:
    collision(buffers::all & b) :
      m_cp { "chunk-collision.comp.spv", { *b.inst, *m_buf.buffer } }
    , m_dcmd { *b.dcmd }
    {}

    void cmd(vee::command_buffer cb) {
      upc pc {};
      m_cp.cmd_bind(cb, &pc, { 0, 1 });
      vee::cmd_dispatch_indirect(cb, m_dcmd);
    }

    auto map() { return voo::memiter<centity>(*m_buf.memory, &m_ecnt); }
  };
}
