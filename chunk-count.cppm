#pragma leco add_shader "chunk-count.comp"
export module chunk:count;
import :buffers;
import :cpipeline;
import traits;
import voo;

using namespace wagen;

namespace chunk {
  export class count {
    struct upc {
      unsigned mdl;
    };

    cpipeline<upc, 4> m_cp;

  public:
    explicit count(buffers & b) :
      m_cp { "chunk-count.comp.spv", { *b.inst, *b.vcmd, *b.ecmd, *b.dcmd } }
    {}
    void cmd(vee::command_buffer cb, unsigned mdl, unsigned elems) {
      upc pc { .mdl = static_cast<unsigned>(mdl) };
      m_cp.cmd_dispatch(cb, &pc, { 0, 1, 2, 3 }, { elems, 1U, 1U });
    }
  };
}
