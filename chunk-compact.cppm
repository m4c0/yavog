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

    cpipeline<dotz::ivec3, 2> m_cp;
    unsigned m_len;

  public:
    compact(VkBuffer from, VkBuffer to, unsigned len) :
      m_cp {
        "chunk-compact.comp.spv",
        vee::specialisation_info { 99, len },
        { from, to }
      }
    , m_len { len }
    {}

    void cmd(vee::command_buffer cb) {
      m_cp.cmd_dispatch(cb, &ivec3_z, { 0, 1 }, { m_len, m_len, 1U });
      m_cp.cmd_dispatch(cb, &ivec3_y, { 1, 1 }, { m_len, 1U, m_len });
      m_cp.cmd_dispatch(cb, &ivec3_x, { 1, 1 }, { 1U, m_len, m_len });
    }
  };
}
