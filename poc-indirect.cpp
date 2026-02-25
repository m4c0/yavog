#pragma leco app
#pragma leco add_shader "poc-model.frag"
#pragma leco add_shader "poc-indirect.vert"
import buffers;
import casein;
import hai;
import models;
import vinyl;
import voo;

using namespace wagen;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct app_stuff {
  voo::device_and_queue dq { "poc-model", casein::native_ptr };
  vee::render_pass rp = voo::single_att_render_pass(dq);

  vee::pipeline_layout pl = vee::create_pipeline_layout();
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .shaders {
      voo::vert_shader("poc-indirect.vert.spv").pipeline_stage(),
      voo::frag_shader("poc-model.frag.spv").pipeline_stage(),
    },
    .bindings = buffers::vk::bindings<buffers::vtx>(),
    .attributes = buffers::vk::attrs {
      &buffers::vtx::pos,
      &buffers::vtx::normal,
      &buffers::vtx::uv,
    },
  });

  buffers::all bufs {
    models::cube::t {},
    models::prism::t {},
    models::corner::t {},
  };
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };
  hai::array<vee::framebuffer> fbs = swc.create_framebuffers(*vv::as()->rp);

  ext_stuff() {}
};

extern "C" void casein_init() {
  casein::window_size = { 600, 600 };

  vv::setup([] {
    auto ext = vv::ss()->swc.extent();
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
      vv::as()->bufs.vtx.cmd_bind(cb);
      vv::as()->bufs.idx.cmd_bind(cb);
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vee::cmd_draw_indexed(cb, vv::as()->bufs.idx.count());
    }

    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::window_title = "poc-indirect";
}
