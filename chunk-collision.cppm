#pragma leco add_shader "chunk-collision.comp"
export module chunk:collision;
import :cpipeline;

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
    unsigned m_elems;

  public:
    collision(VkBuffer buf, unsigned elems) :
      m_cp { "chunk-collision.comp.spv", { buf, *m_buf.buffer } }
    , m_elems { elems }
    {}

    void cmd(vee::command_buffer cb) {
      upc pc {};
      m_cp.cmd_dispatch(cb, &pc, { 0, 1 }, { m_elems, m_ecnt, 1U });
    }
  };
}
