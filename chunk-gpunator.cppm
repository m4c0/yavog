export module chunk:gpunator;
import :bitonic;
import :collision;
import :compact;
import :count;
import :stamp;
import buffers;
import voo;

using namespace wagen;

namespace chunk {
  export struct input {
    buffers::i_buffer inst;
    buffers::vc_buffer vcmd;
    buffers::ec_buffer ecmd;

    template<typename... T> input(unsigned count, T...) :
      inst { count }
    , vcmd { T {}... }
    , ecmd { T {}... }
    {}
  };
  export class gpunator {
    static constexpr const auto vcmd_size = sizeof(VkDrawIndexedIndirectCommand) * model_count;
    static constexpr const auto ecmd_size = sizeof(VkDrawIndirectCommand) * model_count;
    voo::single_cb m_cb { true };

    dotz::ivec3 m_len;

    input m_in;
    buffers::all m_out;

    compact m_comp;
    bitonic m_bit;
    count m_cc;
    collision m_col;

  public:
    template<typename... T> gpunator(dotz::ivec3 len, T...) :
      m_len { len }
    , m_in { static_cast<unsigned>(len.x * len.y * len.z), T {}... }
    , m_out { static_cast<unsigned>(len.x * len.y * len.z), T {}... }
    , m_comp { *m_in.inst, *m_out.inst, len }
    , m_bit { *m_out.inst, static_cast<unsigned>(len.x * len.y * len.z) }
    , m_cc { m_out }
    , m_col { *m_out.inst, static_cast<unsigned>(len.x * len.y * len.z) }
    {
      auto cb = m_cb.cb();

      voo::cmd_buf_sim_use_inherit g { cb };
      m_comp.cmd(cb);
      m_bit.cmd(cb);

      vee::cmd_copy_buffer(cb, *m_in.vcmd, *m_out.vcmd, vcmd_size);
      vee::cmd_copy_buffer(cb, *m_in.ecmd, *m_out.ecmd, ecmd_size);
      for (auto i = 1; i < model_count; i++) {
        m_cc.cmd(cb, i, m_len.x * m_len.y * m_len.z);
      }

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
          vee::memory_barrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT));
    }

    [[nodiscard]] auto stamp() {
      return chunk::stamp { *m_in.inst, m_len };
    }

    void cmd_draw_vtx(vee::command_buffer cb) {
      m_out.cmd_draw_vtx(cb);
    }
    void cmd_draw_edg(vee::command_buffer cb) {
      m_out.cmd_draw_edg(cb);
    }

    void submit() const {
      voo::queue::universal()->submit({ .command_buffer = m_cb.cb() });
    }
  };
}
