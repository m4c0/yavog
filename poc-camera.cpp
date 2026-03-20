#pragma leco app
#pragma leco add_shader "poc-model.frag"
#pragma leco add_shader "poc-camera.vert"
import buffers;
import casein;
import chunk;
import dotz;
import hai;
import models;
import vinyl;
import voo;

using namespace wagen;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct upc {
  dotz::vec2 cam;
} g_pc;

struct app_stuff {
  voo::device_and_queue dq { "poc-indirect", casein::native_ptr };
  vee::render_pass rp = voo::single_att_render_pass(dq);

  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .shaders {
      voo::vert_shader("poc-camera.vert.spv").pipeline_stage(),
      voo::frag_shader("poc-model.frag.spv").pipeline_stage(),
    },
    .bindings = buffers::vk::ibindings(),
    .attributes = buffers::vk::iattrs(),
  });

  buffers::all bufs {
    4,
    models::cube::t {},
  };

  chunk::count cc { *bufs.inst, *bufs.vcmd, *bufs.ecmd };

  app_stuff() {
    auto i = bufs.inst.map();
    i += {
      .pos { 0, -2, 0 },
      .mdl = 1,
      .size { 20, 1, 20 },
    };
    i += {
      .pos { 2, 0, 2 },
      .mdl = 1,
      .size { 1, 1, 1 },
    };

    voo::single_cb cb {};
    { 
      voo::cmd_buf_one_time_submit ots { cb.cb() };
      cc.cmd(cb.cb(), 1, 4);
    }
    voo::queue::universal()->queue_submit({ .command_buffer = cb.cb() });
    vee::device_wait_idle();
  }
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq };
  hai::array<vee::framebuffer> fbs = swc.create_framebuffers(*vv::as()->rp);
};

extern "C" void casein_init() {
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
      vee::cmd_push_vertex_constants(cb, *vv::as()->pl, &g_pc);
      vv::as()->bufs.cmd_draw_vtx(cb);
    }

    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  using namespace casein;
  window_title = "poc-indirect";
  window_size = { 600, 600 };

  handle(MOUSE_MOVE_REL, [] {
    auto [yaw, pitch] = mouse_rel;
    g_pc.cam.x = dotz::clamp(
        g_pc.cam.x - pitch,
        -90.f, 90.f);
    g_pc.cam.y += yaw;
  });
}
