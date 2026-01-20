export module ofs;
import :common;
import clay;
import cube;
import hai;
import texmap;
import traits;
import voo;

using namespace wagen;

namespace ofs {
  export class pipeline : no::no {
    vee::sampler m_light_smp = create_shadowmap_sampler();
    voo::single_frag_dset m_light_dset { 1 };

    vee::pipeline_layout m_pl = vee::create_pipeline_layout({
      .descriptor_set_layouts {{
        *texmap::descriptor_set_layout(),
        m_light_dset.descriptor_set_layout(),
      }},
      .push_constant_ranges {{ vee::vertex_push_constant_range<upc>() }},
    });
    vee::gr_pipeline m_ppl = vee::create_graphics_pipeline({
      .pipeline_layout = *m_pl,
      .render_pass = *create_render_pass(max_sampling()),
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .multisampling = max_sampling(),
      .depth = vee::depth::op_less(),
      .blends {
        vee::colour_blend_classic(),
        vee::colour_blend_none(),
        vee::colour_blend_none(),
      },
      .shaders {
        *clay::vert_shader("poc-mcish", [] {}),
        *clay::frag_shader("poc-mcish", [] {}),
      },
      .bindings {
        cube::v_buffer::vertex_input_bind(),
        cube::i_buffer::vertex_input_bind_per_instance(),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, traits::offset_of(&cube::vtx::pos)),
        vee::vertex_attribute_vec2(0, traits::offset_of(&cube::vtx::uv)),
        vee::vertex_attribute_vec3(0, traits::offset_of(&cube::vtx::normal)),
        vee::vertex_attribute_vec4(1, traits::offset_of(&cube::inst::pos)),
      },
    });

    upc m_pc {};
    vee::extent m_ext {};
    hai::uptr<framebuffer> m_fb {};

  public:
    [[nodiscard]] constexpr const auto & fb() const { return *m_fb; }

    void update_descriptor_sets(vee::image_view::type lmap) {
      vee::update_descriptor_set(m_light_dset.descriptor_set(), 0, lmap, *m_light_smp);
    }
    void setup(const voo::swapchain & swc) {
      m_pc.aspect = swc.aspect();
      m_ext = swc.extent();
      m_fb.reset(new framebuffer { swc.extent() });
    }

    [[nodiscard]] auto cmd_render_pass(vee::command_buffer cb, vee::descriptor_set tmap, float sun_angle) {
      m_pc.sun_angle = sun_angle;
      voo::cmd_render_pass rp { vee::render_pass_begin {
        .command_buffer = cb,
        .render_pass = *m_fb->rp,
        .framebuffer = *m_fb->fb,
        .extent = m_ext,
        .clear_colours { 
          vee::clear_colour({ 0, 0, 0, 1 }), 
          vee::clear_colour({ 0, 0, 10, 0 }), 
          vee::clear_colour({ 0, 0, 0, 0 }), 
          vee::clear_depth(1.0),
          vee::clear_colour({ 0, 0, 0, 1 }), 
          vee::clear_colour({ 0, 0, 10, 0 }), 
          vee::clear_colour({ 0, 0, 0, 0 }), 
          vee::clear_depth(1.0),
        },
      }, true };
      vee::cmd_set_viewport(cb, m_ext);
      vee::cmd_set_scissor(cb, m_ext);
      vee::cmd_bind_gr_pipeline(cb, *m_ppl);
      vee::cmd_push_vertex_constants(cb, *m_pl, &m_pc);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, tmap);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 1, m_light_dset.descriptor_set());
      return rp;
    }
  };
}
