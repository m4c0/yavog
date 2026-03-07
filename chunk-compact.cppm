#pragma leco add_shader "chunk-compact.comp"
export module chunk:compact;
import :bitonic;
import :count;
import :cpipeline;
import :objects;
import dotz;
import voo;

namespace chunk {
  export class compact {
    static constexpr const dotz::ivec3 ivec3_x { 1, 0, 0 };
    static constexpr const dotz::ivec3 ivec3_y { 0, 1, 0 };
    static constexpr const dotz::ivec3 ivec3_z { 0, 0, 1 };

    struct upc {
      dotz::ivec3 n;
      dotz::ivec3 k_len;
    };

    cpipeline<upc, 2> m_cp;
    dotz::ivec3 m_len;

  public:
    compact(VkBuffer from, VkBuffer to, dotz::ivec3 l) :
      m_cp {
        "chunk-compact.comp.spv",
        { from, to }
      }
    , m_len { l }
    {}

    void cmd(vee::command_buffer cb) {
      upc pc { .k_len = m_len };

      pc.n = ivec3_z;
      m_cp.cmd_dispatch(cb, &pc, { 0, 1 }, { m_len.x, m_len.y, 1 });
      pc.n = ivec3_y;
      m_cp.cmd_dispatch(cb, &pc, { 1, 1 }, { m_len.x, 1, m_len.z });
      pc.n = ivec3_x;
      m_cp.cmd_dispatch(cb, &pc, { 1, 1 }, { 1, m_len.y, m_len.z });
    }
  };
}
