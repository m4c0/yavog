export module chunk:gpunator;
import :bitonic;
import :compact;
import :count;
import voo;

using namespace wagen;

namespace chunk {
  export class gpunator {
    static constexpr const auto vcmd_size = sizeof(VkDrawIndexedIndirectCommand) * model_count;
    static constexpr const auto ecmd_size = sizeof(VkDrawIndirectCommand) * model_count;

    struct inst {
      dotz::vec4 rot, pos, size;
    };

    voo::bound_buffer m_local0;
    voo::bound_buffer m_local1;
    voo::bound_buffer m_vcmd = voo::bound_buffer::create_from_host(vcmd_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    voo::bound_buffer m_ecmd = voo::bound_buffer::create_from_host(ecmd_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    unsigned m_len;

    VkBuffer m_vcmd_orig;
    VkBuffer m_ecmd_orig;

    compact m_comp;
    bitonic m_bit;
    count m_cc;

    constexpr auto & output() const {
      return m_bit.use_first_as_output() ? m_local0 : m_local1;
    }

  public:
    struct param {
      unsigned len;
      VkBuffer host;
      VkBuffer vcmd;
      VkBuffer ecmd;
    };

    explicit gpunator(const param & p) :
      m_local0 {
      voo::bound_buffer::create_from_host(
          sizeof(inst) * p.len * p.len * p.len,
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    }
    , m_local1 {
      voo::bound_buffer::create_from_host(
          sizeof(inst) * p.len * p.len * p.len,
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    }
    , m_len { p.len }
    , m_vcmd_orig { p.vcmd }
    , m_ecmd_orig { p.ecmd }
    , m_comp { p.host, *m_local0.buffer, *m_local1.buffer, p.len }
    , m_bit { *m_local0.buffer, *m_local1.buffer, p.len * p.len * p.len }
    , m_cc { *output().buffer, *m_vcmd.buffer, *m_ecmd.buffer }
    {}

    constexpr auto vcmd() const { return *m_vcmd.buffer; }
    constexpr auto ecmd() const { return *m_ecmd.buffer; }
    constexpr auto insts() const { return *output().buffer; }

    constexpr auto vcmd_memory() const { return *m_vcmd.memory; }
    constexpr auto ecmd_memory() const { return *m_ecmd.memory; }
    constexpr auto output_memory() const { return *output().memory; }

    void cmd(vee::command_buffer cb) {
      m_comp.cmd(cb);
      m_bit.cmd(cb);

      vee::cmd_copy_buffer(cb, m_vcmd_orig, *m_vcmd.buffer, vcmd_size);
      vee::cmd_copy_buffer(cb, m_ecmd_orig, *m_ecmd.buffer, ecmd_size);
      for (auto i = 1; i < model_count; i++) {
        m_cc.cmd(cb, i, m_len * m_len * m_len);
      }
    }
  };
}
