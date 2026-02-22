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

    cpipeline<upc, 2> m_cp;
    unsigned m_elems;

  public:
    bitonic(VkBuffer a, VkBuffer b, unsigned elems) :
      m_cp { "chunk-bitonic.comp.spv", { a, b } }
    , m_elems { elems }
    {}

    /// Returns "true" if output will be on buffer "a"
    constexpr bool use_first_as_output() const {
      bool res = true;
      for (unsigned jump = 2; jump <= m_elems; jump <<= 1) {
        for (unsigned div = jump; div >= 2; div >>= 1) {
          res = !res;
        }
      }
      return res;
    }

    void cmd(vee::command_buffer cb) {
      bool use01 = true;
      for (unsigned jump = 2; jump <= m_elems; jump <<= 1) {
        for (unsigned div = jump; div >= 2; div >>= 1) {
          upc pc = { .jump = jump, .div = div };
          if (use01) m_cp.cmd_dispatch(cb, &pc, { 0, 1 }, { m_elems, 1U, 1U });
          else       m_cp.cmd_dispatch(cb, &pc, { 1, 0 }, { m_elems, 1U, 1U });
          use01 = !use01;
        }
      }
    }
  };
}
