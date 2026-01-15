export module post;
import clay;
import hai;
import ofs;
import voo;

namespace post {
  export class pipeline {
    // TODO: change sampler to remove edge-wrapping
    vee::sampler m_smp = vee::create_sampler(vee::linear_sampler);

    vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({
      vee::dsl_fragment_sampler(),
      vee::dsl_fragment_sampler(),
      vee::dsl_fragment_sampler(),
      vee::dsl_fragment_sampler(),
    });
    vee::descriptor_pool m_dpool = vee::create_descriptor_pool(1, {
      vee::combined_image_sampler(4)
    });
    vee::descriptor_set m_dset = vee::allocate_descriptor_set(*m_dpool, *m_dsl);

    vee::pipeline_layout m_pl = vee::create_pipeline_layout(
        *m_dsl,
        vee::fragment_push_constant_range<vee::extent>());
    vee::render_pass m_rp;
    vee::gr_pipeline m_ppl;

    hai::array<vee::framebuffer> m_fbs {};

  public:
    pipeline(voo::device_and_queue & dq) :
      m_rp { voo::single_att_render_pass(dq) }
    , m_ppl { vee::create_graphics_pipeline({
        .pipeline_layout = *m_pl,
        .render_pass = *m_rp,
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

    void setup(voo::swapchain & swc) {
      m_fbs = swc.create_framebuffers(*m_rp);
    }
    void update_descriptor_sets(const ofs::framebuffer & ofs) {
      vee::update_descriptor_set(m_dset, 0, *ofs.colour.iv,   *m_smp);
      vee::update_descriptor_set(m_dset, 1, *ofs.position.iv, *m_smp);
      vee::update_descriptor_set(m_dset, 2, *ofs.normal.iv,   *m_smp);
      vee::update_descriptor_set(m_dset, 3, *ofs.depth.iv,    *m_smp);
    }

    void render(vee::command_buffer cb, voo::swapchain & swc) {
      auto ext = swc.extent();

      voo::cmd_render_pass rpg {vee::render_pass_begin{
        .command_buffer = cb,
        .render_pass = *m_rp,
        .framebuffer = *m_fbs[swc.index()],
        .extent = ext,
        .clear_colours { vee::clear_colour({}) },
      }, true};

      vee::cmd_set_viewport(cb, ext);
      vee::cmd_push_fragment_constants(cb, *m_pl, &ext);
      vee::cmd_bind_gr_pipeline(cb, *m_ppl);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset);
      vee::cmd_draw(cb, 4);
    }
  };
}

#pragma leco add_shader "post.frag"
#pragma leco add_shader "post.vert"
