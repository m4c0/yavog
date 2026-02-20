#pragma leco add_shader "chunk-bitonic.comp"
export module chunk:bitonic;
import voo;

using namespace wagen;

namespace chunk {
  export class bitonic {
    struct upc {
      unsigned jump;
      unsigned div;
    };

    vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({
      vee::dsl_compute_storage(),
      vee::dsl_compute_storage(),
    });
    vee::descriptor_pool m_dpool = vee::create_descriptor_pool(2, {
      vee::storage_buffer(4),
    });

    vee::pipeline_layout m_pl = vee::create_pipeline_layout(*m_dsl, vee::compute_push_constant_range<upc>());
    vee::c_pipeline m_ppl = vee::create_compute_pipeline(*m_pl, *voo::comp_shader("chunk-bitonic.comp.spv"), "main");

    vee::descriptor_set m_dset_a;
    vee::descriptor_set m_dset_b;

  public:
    bitonic(vee::descriptor_set da, vee::descriptor_set db) :
      m_dset_a { da }
    , m_dset_b { db } {}

    /// Returns "true" if output will be on buffer "a"
    bool cmd(vee::command_buffer cb, unsigned elems) {
      vee::cmd_bind_c_pipeline(cb, *m_ppl);

      bool use01 = true;
      for (unsigned jump = 2; jump <= elems; jump <<= 1) {
        for (unsigned div = jump; div >= 2; div >>= 1) {
          upc pc = { .jump = jump, .div = div };
          vee::cmd_bind_c_descriptor_set(cb, *m_pl, 0, use01 ? m_dset_a : m_dset_b);
          vee::cmd_push_compute_constants(cb, *m_pl, &pc);
          vee::cmd_dispatch(cb, elems, 1, 1);
          use01 = !use01;
        }
      }
      return use01;
    }
  };
}
