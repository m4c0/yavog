#pragma leco add_shader "chunk-bitonic.comp"
export module chunk:bitonic;
import :cpipeline;
import voo;

using namespace wagen;

namespace chunk {
  export class bitonic {
    struct upc {
      unsigned jump;
      unsigned div;
    };

    cpipeline<upc, 1> m_cp;
    unsigned m_elems;

  public:
    bitonic(VkBuffer buf, unsigned elems) :
      m_cp { "chunk-bitonic.comp.spv", { buf } }
    , m_elems { elems }
    {}

    void cmd(vee::command_buffer cb) {
      for (unsigned jump = 2; jump <= m_elems; jump <<= 1) {
        for (unsigned div = jump; div >= 2; div >>= 1) {
          upc pc = { .jump = jump, .div = div };
          m_cp.cmd_dispatch(cb, &pc, { 0 }, { m_elems / 32, 1U, 1U });
        }
      }
    }
  };
}
