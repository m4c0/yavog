#pragma leco add_shader "chunk-count.comp"
export module chunk:count;
import :cpipeline;
import traits;
import voo;

using namespace wagen;

namespace chunk {
  export class count {
    struct upc {
      unsigned mdl;
    };

    cpipeline<upc, 3> m_cp;

  public:
    explicit count(VkBuffer in, VkBuffer vcmd, VkBuffer ecmd) :
      m_cp { "chunk-count.comp.spv", { in, vcmd, ecmd } }
    {}
    void cmd(vee::command_buffer cb, unsigned mdl, unsigned elems) {
      upc pc { .mdl = static_cast<unsigned>(mdl) };

      m_cp.cmd_bind(cb, &pc, { 0, 1, 2 });
      vee::cmd_dispatch(cb, elems, 1, 1);
    }
  };
}
