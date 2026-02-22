#pragma leco add_shader "chunk-count.comp"
export module chunk:count;
import traits;
import voo;

using namespace wagen;

namespace chunk {
  export class count {
    struct upc {
      unsigned mdl;
    };

    vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({
      vee::dsl_compute_storage(),
      vee::dsl_compute_storage(),
      vee::dsl_compute_storage(),
    });
    vee::descriptor_pool m_dpool = vee::create_descriptor_pool(1, {
      vee::storage_buffer(3),
    });

    vee::pipeline_layout m_pl = vee::create_pipeline_layout(*m_dsl, vee::compute_push_constant_range<upc>());
    vee::c_pipeline m_ppl = vee::create_compute_pipeline(*m_pl, *voo::comp_shader("chunk-count.comp.spv"), "main");

    vee::descriptor_set m_dset = vee::allocate_descriptor_set(*m_dpool, *m_dsl);;

  public:
    explicit count(VkBuffer in, VkBuffer vcmd, VkBuffer ecmd) {
      vee::update_descriptor_set(m_dset, 0, in);
      vee::update_descriptor_set(m_dset, 1, vcmd);
      vee::update_descriptor_set(m_dset, 2, ecmd);
    }
    void cmd(vee::command_buffer cb, unsigned mdl, unsigned elems) {
      upc pc { .mdl = static_cast<unsigned>(mdl) };

      vee::cmd_bind_c_pipeline(cb, *m_ppl);
      vee::cmd_bind_c_descriptor_set(cb, *m_pl, 0, m_dset);
      vee::cmd_push_compute_constants(cb, *m_pl, &pc);
      vee::cmd_dispatch(cb, elems, 1, 1);
    }
  };
}
