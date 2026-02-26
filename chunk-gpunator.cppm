export module chunk:gpunator;
import :bitonic;
import :compact;
import :count;
import buffers;
import voo;

using namespace wagen;

namespace chunk {
  export struct input {
    buffers::i_buffer insts;
    buffers::vc_buffer vcmd;
    buffers::ec_buffer ecmd;

    template<typename... T> input(unsigned count, T...) :
      insts { count }
    , vcmd { T {}... }
    , ecmd { T {}... }
    {}
  };
  export class gpunator {
    static constexpr const auto vcmd_size = sizeof(VkDrawIndexedIndirectCommand) * model_count;
    static constexpr const auto ecmd_size = sizeof(VkDrawIndirectCommand) * model_count;
    voo::single_cb m_cb { false };

    voo::bound_buffer m_local0;
    voo::bound_buffer m_local1;

    unsigned m_len;

    input * m_in;
    buffers::all * m_out;

    compact m_comp;
    bitonic m_bit;
    count m_cc;

    constexpr auto & output() const {
      return m_bit.use_first_as_output() ? m_local0 : m_local1;
    }

  public:
    struct param {
      unsigned len;
      input * in;
      buffers::all * out;
    };

    explicit gpunator(const param & p) :
      m_local0 {
      voo::bound_buffer::create_from_host(
          sizeof(buffers::inst) * p.len * p.len * p.len,
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    }
    , m_local1 {
      voo::bound_buffer::create_from_host(
          sizeof(buffers::inst) * p.len * p.len * p.len,
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    }
    , m_len { p.len }
    , m_in { p.in }
    , m_out { p.out }
    , m_comp { *p.in->insts, *m_local0.buffer, *m_local1.buffer, p.len }
    , m_bit { *m_local0.buffer, *m_local1.buffer, p.len * p.len * p.len }
    , m_cc { *output().buffer, *m_out->vcmd, *m_out->ecmd }
    {
      auto cb = m_cb.cb();

      voo::cmd_buf_sim_use_inherit g { cb };
      m_comp.cmd(cb);
      m_bit.cmd(cb);

      vee::cmd_copy_buffer(cb, *m_in->vcmd, *m_out->vcmd, vcmd_size);
      vee::cmd_copy_buffer(cb, *m_in->ecmd, *m_out->ecmd, ecmd_size);
      for (auto i = 1; i < model_count; i++) {
        m_cc.cmd(cb, i, m_len * m_len * m_len);
      }
    }

    constexpr auto command_buffer() const { return m_cb.cb(); }
  };
}
