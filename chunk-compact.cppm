#pragma leco add_shader "chunk-compact.comp"
export module chunk:compact;
import :bitonic;
import :count;
import dotz;
import voo;

namespace chunk {
  export class compact {
    static constexpr const dotz::ivec3 ivec3_x { 1, 0, 0 };
    static constexpr const dotz::ivec3 ivec3_y { 0, 1, 0 };
    static constexpr const dotz::ivec3 ivec3_z { 0, 0, 1 };

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

    bitonic m_bit;
    count m_cc;

    vee::pipeline_layout m_pl = vee::create_pipeline_layout(*m_dsl, vee::compute_push_constant_range<dotz::ivec3>());
    vee::c_pipeline m_ppl;
    unsigned m_len;

  public:
    compact(unsigned len, VkBuffer host, VkBuffer local0, VkBuffer local1) :
      m_bit { m_dset01, m_dset10, len * len * len }
    , m_cc { host, nullptr }
    , m_ppl {
        vee::create_compute_pipeline(
            *m_pl, *voo::comp_shader("chunk-compact.comp.spv"),
            "main", vee::specialisation_info { 99, len })
      }
    , m_len { len }
    {
      vee::update_descriptor_set(m_dset_hl, 0, host);
      vee::update_descriptor_set(m_dset_hl, 1, local0);

      vee::update_descriptor_set(m_dset01, 0, local0);
      vee::update_descriptor_set(m_dset01, 1, local1);

      vee::update_descriptor_set(m_dset10, 0, local1);
      vee::update_descriptor_set(m_dset10, 1, local0);
    }

    bool cmd(vee::command_buffer cb) {
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

      m_cc.cmd(cb, 2, m_len * m_len * m_len);

      return m_bit.cmd(cb);
    }
  };
}
