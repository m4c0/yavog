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

    vee::pipeline_layout m_pl = vee::create_pipeline_layout(*m_dsl, vee::compute_push_constant_range<upc>());
    vee::c_pipeline m_ppl = vee::create_compute_pipeline(*m_pl, *voo::comp_shader("chunk-bitonic.comp.spv"), "main");

    vee::descriptor_set m_dset_a;
    vee::descriptor_set m_dset_b;

    unsigned m_elems;

  public:
    bitonic(vee::descriptor_set da, vee::descriptor_set db, unsigned elems) :
      m_dset_a { da }
    , m_dset_b { db }
    , m_elems { elems }
    {}

    /// Returns "true" if output will be on buffer "a"
    constexpr bool use_first_as_output() const {
      bool res = true;
      for (unsigned jump = 2; jump <= m_elems; jump <<= 1) {
        for (unsigned div = jump; div >= 2; div >>= 1) {
          res = !res;
        }
      }
      return res;
    }

    void cmd(vee::command_buffer cb) {
      vee::cmd_bind_c_pipeline(cb, *m_ppl);

      bool use01 = true;
      for (unsigned jump = 2; jump <= m_elems; jump <<= 1) {
        for (unsigned div = jump; div >= 2; div >>= 1) {
          upc pc = { .jump = jump, .div = div };
          vee::cmd_bind_c_descriptor_set(cb, *m_pl, 0, use01 ? m_dset_a : m_dset_b);
          vee::cmd_push_compute_constants(cb, *m_pl, &pc);
          vee::cmd_dispatch(cb, m_elems, 1, 1);
          use01 = !use01;
        }
      }
    }
  };
}
