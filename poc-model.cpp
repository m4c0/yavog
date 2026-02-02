#pragma leco app
#pragma leco add_shader "poc-model.frag"
#pragma leco add_shader "poc-model.vert"
import casein;
import clay;
import cube;
import dotz;
import hai;
import ofs;
import sitime;
import traits;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct app_stuff : vinyl::base_app_stuff {
  cube::drawer cube {};

  vee::render_pass rp = voo::single_att_render_pass(dq);

  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<float>());
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
    .shaders {
      voo::vert_shader("poc-model.vert.spv").pipeline_stage(),
      voo::frag_shader("poc-model.frag.spv").pipeline_stage(),
    },
    .bindings {
      vee::vertex_input_bind(sizeof(dotz::vec4)),
    },
    .attributes {
      vee::vertex_attribute_vec4(0, 0),
    },
  });

  clay::buffer<dotz::vec4> vtx { 8 };

  app_stuff() : base_app_stuff { "poc-model" } {
    auto m = vtx.map();
    for (auto v : cube::vtxes) m += v;
  }
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };
  hai::array<vee::framebuffer> fbs = swc.create_framebuffers(*vv::as()->rp);

  ext_stuff() {}
};

static sitime::stopwatch g_tt {};
extern "C" void casein_init() {
  casein::window_size = { 600, 600 };

  vv::setup([] {
    auto ext = vv::ss()->swc.extent();
    auto angle = g_tt.secs() * 10;

    vv::ss()->swc.acquire_next_image();

    auto cb = vv::ss()->cb.cb();
    {
      voo::cmd_buf_one_time_submit ots { cb };
      voo::cmd_render_pass rp { vee::render_pass_begin {
        .command_buffer = cb,
        .render_pass = *vv::as()->rp,
        .framebuffer = *vv::ss()->fbs[vv::ss()->swc.index()],
        .extent = ext,
        .clear_colours { 
          vee::clear_colour({ 0, 0, 0, 1 }),
        },
      }, true };
      vee::cmd_set_viewport(cb, ext);
      vee::cmd_set_scissor(cb, ext);
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vee::cmd_push_vertex_constants(cb, *vv::as()->pl, &angle);
      vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->vtx, 0);
      vee::cmd_draw(cb, 8);
    }

    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });
}
