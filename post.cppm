export module post;
import clay;
import voo;

namespace post {
  export class pipeline {
    vee::sampler m_smp = vee::create_sampler(vee::linear_sampler);
    voo::single_frag_dset m_dset { 1 };
    vee::pipeline_layout m_pl = vee::create_pipeline_layout(m_dset.descriptor_set_layout());
    vee::gr_pipeline m_ppl;

  public:
    pipeline(voo::device_and_queue & dq) :
      m_ppl { vee::create_graphics_pipeline({
        .pipeline_layout = *m_pl,
        .render_pass = *voo::single_att_render_pass(dq),
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .back_face_cull = false,
        .shaders {
          *clay::vert_shader("post", [] {}),
          *clay::frag_shader("post", [] {}),
        },
        .bindings {},
        .attributes {},
      }) }
    {}

    void update_descriptor_set(vee::image_view::type input) {
      vee::update_descriptor_set(m_dset.descriptor_set(), 0, input, *m_smp);
    }

    void render(vee::command_buffer cb, voo::swapchain_and_stuff & sw) {
      auto rp = sw.cmd_render_pass({
        .clear_colours { 
          vee::clear_colour({}), 
          vee::clear_depth(1.0),
        },
      });

      vee::cmd_set_viewport(cb, sw.extent());
      vee::cmd_bind_gr_pipeline(cb, *m_ppl);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset.descriptor_set());
      vee::cmd_draw(cb, 4);
    }
  };
}

#pragma leco add_shader "post.frag"
#pragma leco add_shader "post.vert"
