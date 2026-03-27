#pragma leco add_shader "chunk-collision.comp"
export module chunk:collision;
import :cpipeline;
import buffers;

namespace chunk {
  static inline constexpr const auto max_collisions = 8;

  export struct centity {
    dotz::vec3 pos;
    float hit;
  };
  static_assert(sizeof(centity) == 16);

  export class collision {
    struct upc {
      unsigned x;
    };

    voo::bound_buffer m_buf = voo::bound_buffer::create_from_host(
        max_collisions * sizeof(centity),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    cpipeline<upc, 2> m_cp;

    VkBuffer m_dcmd;

  public:
    collision(buffers::all & b) :
      m_cp { "chunk-collision.comp.spv", { *b.inst, *m_buf.buffer } }
    , m_dcmd { *b.dcmd }
    {}

    void cmd_dispatch(vee::command_buffer cb) {
      upc pc {};
      m_cp.cmd_dispatch_indirect(cb, &pc, { 0, 1 }, m_dcmd);
    }

    void cmd_reset(vee::command_buffer cb) {
      vee::cmd_update_buffer(cb, m_dcmd, VkDispatchIndirectCommand { 1, max_collisions, 1 });
    }

    auto map() { return voo::memiter<centity>(*m_buf.memory); }
  };
}
