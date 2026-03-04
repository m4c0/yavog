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
    dotz::ivec3 m_len;

  public:
    compact(VkBuffer from, VkBuffer to, dotz::ivec3 l) :
      m_cp {
        "chunk-compact.comp.spv",
        vee::specialisation_info<dotz::ivec3>(l, {
          vee::specialisation_map_entry(94, &dotz::ivec3::x),
          vee::specialisation_map_entry(95, &dotz::ivec3::y),
          vee::specialisation_map_entry(96, &dotz::ivec3::z),
        }),
        { from, to }
      }
    , m_len { l }
    {}

    void cmd(vee::command_buffer cb) {
      m_cp.cmd_dispatch(cb, &ivec3_z, { 0, 1 }, { m_len.x, m_len.y, 1 });
      m_cp.cmd_dispatch(cb, &ivec3_y, { 1, 1 }, { m_len.x, 1, m_len.z });
      m_cp.cmd_dispatch(cb, &ivec3_x, { 1, 1 }, { 1, m_len.y, m_len.z });
    }
  };
}
