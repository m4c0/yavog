#pragma leco add_shader "chunk-compact.comp"
export module chunk:compact;
import :bitonic;
import :count;
import :objects;
import dotz;
import voo;

namespace chunk {
  export class compact {
    static constexpr const dotz::ivec3 ivec3_x { 1, 0, 0 };
    static constexpr const dotz::ivec3 ivec3_y { 0, 1, 0 };
    static constexpr const dotz::ivec3 ivec3_z { 0, 0, 1 };

    static constexpr const auto vcmd_size = sizeof(VkDrawIndexedIndirectCommand) * model_count;
    static constexpr const auto ecmd_size = sizeof(VkDrawIndirectCommand) * model_count;

    struct inst {
      dotz::vec4 rot, pos, size;
    };

    vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({
      vee::dsl_compute_storage(),
      vee::dsl_compute_storage(),
    });
    vee::descriptor_pool m_dpool = vee::create_descriptor_pool(3, {
      vee::storage_buffer(6),
    });
    vee::descriptor_set m_dset_hl = vee::allocate_descriptor_set(*m_dpool, *m_dsl);
    vee::descriptor_set m_dset01 = vee::allocate_descriptor_set(*m_dpool, *m_dsl);
    vee::descriptor_set m_dset10 = vee::allocate_descriptor_set(*m_dpool, *m_dsl);

    voo::bound_buffer m_local0;
    voo::bound_buffer m_local1;
    voo::bound_buffer m_vcmd = voo::bound_buffer::create_from_host(vcmd_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    voo::bound_buffer m_ecmd = voo::bound_buffer::create_from_host(ecmd_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    vee::pipeline_layout m_pl = vee::create_pipeline_layout(*m_dsl, vee::compute_push_constant_range<dotz::ivec3>());
    vee::c_pipeline m_ppl;
    unsigned m_len;

    VkBuffer m_vcmd_orig;
    VkBuffer m_ecmd_orig;

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

    compact(const param & p) :
      m_local0 {
      voo::bound_buffer::create_from_host(
          sizeof(inst) * p.len * p.len * p.len,
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
    }
    , m_local1 {
      voo::bound_buffer::create_from_host(
          sizeof(inst) * p.len * p.len * p.len,
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
    }
    , m_ppl {
        vee::create_compute_pipeline(
            *m_pl, *voo::comp_shader("chunk-compact.comp.spv"),
            "main", vee::specialisation_info { 99, p.len })
      }
    , m_len { p.len }
    , m_vcmd_orig { p.vcmd }
    , m_ecmd_orig { p.ecmd }
    , m_bit { m_dset01, m_dset10, p.len * p.len * p.len }
    , m_cc { *output().buffer, *m_vcmd.buffer, *m_ecmd.buffer }
    {
      vee::update_descriptor_set(m_dset_hl, 0, p.host);
      vee::update_descriptor_set(m_dset_hl, 1, *m_local0.buffer);

      vee::update_descriptor_set(m_dset01, 0, *m_local0.buffer);
      vee::update_descriptor_set(m_dset01, 1, *m_local1.buffer);

      vee::update_descriptor_set(m_dset10, 0, *m_local1.buffer);
      vee::update_descriptor_set(m_dset10, 1, *m_local0.buffer);
    }

    constexpr auto vcmd_memory() const { return *m_vcmd.memory; }
    constexpr auto ecmd_memory() const { return *m_ecmd.memory; }
    constexpr auto output_memory() const {
      return *output().memory;
    }

    void cmd(vee::command_buffer cb) {
      vee::cmd_bind_c_pipeline(cb, *m_ppl);
      vee::cmd_bind_c_descriptor_set(cb, *m_pl, 0, m_dset_hl);
      vee::cmd_push_compute_constants(cb, *m_pl, &ivec3_z);
      vee::cmd_dispatch(cb, m_len, m_len, 1);

      vee::cmd_bind_c_descriptor_set(cb, *m_pl, 0, m_dset01);
      vee::cmd_push_compute_constants(cb, *m_pl, &ivec3_y);
      vee::cmd_dispatch(cb, m_len, 1, m_len);

      vee::cmd_bind_c_descriptor_set(cb, *m_pl, 0, m_dset10);
      vee::cmd_push_compute_constants(cb, *m_pl, &ivec3_x);
      vee::cmd_dispatch(cb, 1, m_len, m_len);

      m_bit.cmd(cb);

      vee::cmd_copy_buffer(cb, m_vcmd_orig, *m_vcmd.buffer, vcmd_size);
      vee::cmd_copy_buffer(cb, m_ecmd_orig, *m_ecmd.buffer, ecmd_size);
      for (auto i = 1; i < model_count; i++) {
        m_cc.cmd(cb, i, m_len * m_len * m_len);
      }
    }
  };
}
