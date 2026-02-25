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
    .bindings = buffers::vk::ibindings(),
    .attributes = buffers::vk::iattrs(),
  });

  buffers::all bufs {
    models::cube::t {},
    models::prism::t {},
    models::corner::t {},
  };
  buffers::i_buffer insts { 4 };

  app_stuff() {
    auto i = insts.map();
    i += {
      .pos { 1, 1, 4 },
      .size { 1 },
    };
    i += {
      .pos { -1, 1, 4 },
      .size { 1 },
    };
    i += {
      .pos { 1, -1, 4 },
      .size { 1 },
    };
    i += {
      .pos { -1, -1, 4 },
      .size { 1 },
    };

    auto vc = bufs.vcmd.map(nullptr);
    vc[1].instanceCount = 1;
    vc[2].firstInstance = 1;
    vc[2].instanceCount = 2;
    vc[3].firstInstance = 3;
    vc[3].instanceCount = 1;
  }
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
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vv::as()->bufs.vtx.cmd_bind(cb);
      vv::as()->bufs.idx.cmd_bind(cb);
      vv::as()->insts.cmd_bind(cb);
      vv::as()->bufs.vcmd.cmd_draw(cb);
    }

    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::window_title = "poc-indirect";
}
