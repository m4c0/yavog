#pragma leco add_shader "chunk-collision.comp"
export module chunk:collision;
import :cpipeline;

namespace chunk {
  export class collision {
    struct upc {
      unsigned x;
    };
    
    cpipeline<upc, 1> m_cp;
    unsigned m_elems;

  public:
    collision(VkBuffer buf, unsigned elems) :
      m_cp { "chunk-collision.comp.spv", { buf } }
    , m_elems { elems }
    {}

    void cmd(vee::command_buffer cb) {
      upc pc {};
      m_cp.cmd_dispatch(cb, &pc, { 0 }, { m_elems, 1U, 1U });
    }
  };
}
