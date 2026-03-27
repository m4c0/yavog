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
  export class gpunator {
    static constexpr const auto vcmd_size = sizeof(VkDrawIndexedIndirectCommand) * model_count;
    static constexpr const auto ecmd_size = sizeof(VkDrawIndirectCommand) * model_count;
    voo::single_cb m_cb { true };

    dotz::ivec3 m_len;

    ::buffers::i_buffer m_in;
    buffers m_out;

    compact m_comp;
    bitonic m_bit;
    count m_cc;
    collision m_col;

    template<typename T> void update_buffers(VkCommandBuffer cb, unsigned & i, unsigned & fi, int & vofs, unsigned & fv) {
      m_out.vcmd.update_buffer(cb, {
        .indexCount   = ::buffers::size(T::tri) * 3,
        .firstIndex   = fi,
        .vertexOffset = vofs,
      }, i);
      m_out.ecmd.update_buffer(cb, {
        .vertexCount  = ::buffers::size(T::edg) * 3,
        .firstVertex  = fv,
      }, i);
      i++;
      fi   += ::buffers::size(T::tri) * 3;
      vofs += ::buffers::size(T::vtx);
      fv   += ::buffers::size(T::edg) * 3;
    }

    // TODO: move this back to "chunk::buffers"
    template<typename... T> void update_buffers(VkCommandBuffer cb) {
      m_out.vcmd.update_buffer(cb, {}, 0);
      m_out.ecmd.update_buffer(cb, {}, 0);

      unsigned i    = 1;
      unsigned fi   = 0;
      int      vofs = 0;
      unsigned fv   = 0;
      (update_buffers<T>(cb, i, fi, vofs, fv), ...);
    }

  public:
    template<typename... T> gpunator(dotz::ivec3 len, T...) :
      m_len { len }
    , m_in { static_cast<unsigned>(len.x * len.y * len.z) }
    , m_out { static_cast<unsigned>(len.x * len.y * len.z), T {}... }
    , m_comp { *m_in, *m_out.inst, len }
    , m_bit { *m_out.inst, static_cast<unsigned>(len.x * len.y * len.z) }
    , m_cc { m_out }
    , m_col { m_out }
    {
      auto cb = m_cb.cb();

      voo::cmd_buf_sim_use_inherit g { cb };
      m_comp.cmd(cb);
      m_bit.cmd(cb);
      m_col.cmd_reset(cb);
      update_buffers<T...>(cb);
      for (auto i = 1; i < model_count; i++) {
        m_cc.cmd(cb, i, m_len.x * m_len.y * m_len.z);
      }

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
          vee::memory_barrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT));
    }

    [[nodiscard]] auto stamp() {
      return chunk::stamp { *m_in, m_len };
    }
    [[nodiscard]] auto collision() {
      return chunk::collision { m_out };
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
