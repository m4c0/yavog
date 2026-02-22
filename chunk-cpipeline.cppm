export module chunk:cpipeline;
import hai;
import sv;
import voo;

using namespace wagen;

namespace chunk {
  template<typename U, unsigned N, unsigned M = N>
  class cpipeline {
    vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({ vee::dsl_compute_storage() });
    vee::descriptor_pool m_dpool = vee::create_descriptor_pool(M, { vee::storage_buffer(M) });
    hai::array<vee::descriptor_set> m_dsets { M };

    vee::pipeline_layout m_pl = vee::create_pipeline_layout({
      .descriptor_set_layouts = [this] {
        hai::array<VkDescriptorSetLayout> dsls { N };
        for (auto & dsl : dsls) dsl = *m_dsl;
        return dsls;
      }(),
      .push_constant_ranges {{ vee::compute_push_constant_range<U>() }},
    });
    vee::c_pipeline m_ppl;

  public:
    cpipeline(sv shader, const VkBuffer (&bufs)[M]) :
      m_ppl { vee::create_compute_pipeline(*m_pl, *voo::comp_shader(shader), "main") }
    {
      for (auto i = 0; i < M; i++) {
        m_dsets[i] = vee::allocate_descriptor_set(*m_dpool, *m_dsl);
        vee::update_descriptor_set(m_dsets[i], 0, bufs[i]);
      }
    }

    cpipeline(sv shader, const auto & si, const VkBuffer (&bufs)[M]) :
      m_ppl { vee::create_compute_pipeline(*m_pl, *voo::comp_shader(shader), "main", si) }
    {
      for (auto i = 0; i < M; i++) {
        m_dsets[i] = vee::allocate_descriptor_set(*m_dpool, *m_dsl);
        vee::update_descriptor_set(m_dsets[i], 0, bufs[i]);
      }
    }

    void cmd_bind(vee::command_buffer cb, const U * pc, const unsigned (&bufs)[N]) {
      vee::cmd_bind_c_pipeline(cb, *m_ppl);
      vee::cmd_push_compute_constants(cb, *m_pl, pc);
      for (auto i = 0; i < N; i++) vee::cmd_bind_c_descriptor_set(cb, *m_pl, i, m_dsets[bufs[i]]);
    }
  };
}
