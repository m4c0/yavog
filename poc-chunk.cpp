#pragma leco app
#pragma leco add_shader "poc-model.frag"
#pragma leco add_shader "poc-indirect.vert"
#pragma leco add_resource_dir assets

import buffers;
import casein;
import chunk;
import dotz;
import embedded;
import hai;
import indirectdebug;
import models;
import ofs;
import post;
import silog;
import sitime;
import sv;
import texmap;
import timing;
import traits;
import vinyl;
import voo;
import wagen;

using namespace wagen;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct app_stuff {
  voo::device_and_queue dq { "poc-chunk", casein::native_ptr};
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

  voo::single_cb scb {};

  static constexpr const unsigned len = 32; // PoT padding
  static constexpr const unsigned count = len * len * len; // PoT padding
  static_assert(chunk::len <= len);

  texmap::cache tmap {};

  buffers::all bufs {
    count,
    models::corner::t {},
    models::cube::t {},
    models::prism::t {},
  };
  chunk::input ch_in {
    count,
    models::corner::t {},
    models::cube::t {},
    models::prism::t {},
  };

  chunk::t ch {};
  chunk::gpunator cgpu {{
    .len = len,
    .in = &ch_in,
    .out = &bufs,
  }};

  app_stuff() {
    using enum chunk::model;
    auto dirt  = tmap.load("Ground105_1K-JPG_Color.jpg");
    auto grass = tmap.load("Ground037_1K-JPG_Color.jpg");

    constexpr const auto mm = chunk::minmax;
    for (auto x = -mm; x <= mm; x++) {
      for (auto y = -mm; y <= -2; y++) {
        for (auto z = -mm; z <= mm; z++) {
          ch.set({ x, y, z }, { .mdl = cube, .txt = dirt });
        }
      }
    }
    const dotz::vec4 r90 { 0, 0.7071, 0, 0.7071 };
    const dotz::vec4 r180 { 0, 1, 0, 0 };
    const dotz::vec4 r270 { 0, -0.7071, 0, 0.7071 };
    for (auto x = -1; x <= 1; x++) {
      for (auto y = -1; y <= 1; y++) {
        ch.set({ 3 + x, -1, 5 + y }, { .mdl = cube, .txt = grass });
      }
      ch.set({ 5, -1, 5 + x }, {              .mdl = prism, .txt = grass });
      ch.set({ 1, -1, 5 + x }, { .rot = r180, .mdl = prism, .txt = grass });
      ch.set({ 3 + x, -1, 3 }, { .rot = r90,  .mdl = prism, .txt = grass });
      ch.set({ 3 + x, -1, 7 }, { .rot = r270, .mdl = prism, .txt = grass });
    }
    ch.set({ 5, -1, 7 }, {              .mdl = corner, .txt = grass });
    ch.set({ 1, -1, 7 }, { .rot = r270, .mdl = corner, .txt = grass });
    ch.set({ 5, -1, 3 }, { .rot = r90,  .mdl = corner, .txt = grass });
    ch.set({ 1, -1, 3 }, { .rot = r180, .mdl = corner, .txt = grass });

    {
      auto m = ch_in.inst.map();
      ch.copy(m, { 0, 0, 16 }, len);
    }

    {
      auto cb = scb.cb();

      voo::cmd_buf_one_time_submit ots { cb };

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
          vee::memory_barrier(0, 0));

      vee::cmd_execute_command(cb, cgpu.command_buffer());

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
          vee::memory_barrier(0, 0));
    }

    voo::queue::universal()->submit({ .command_buffer = scb.cb() });
    indirectdebug::dump(bufs, count, 4);
  }
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };
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
      vv::as()->bufs.cmd_draw_vtx(cb);
    }

    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::handle(casein::KEY_DOWN, casein::K_F, [] {
    casein::fullscreen = !casein::fullscreen;
    casein::interrupt(casein::IRQ_FULLSCREEN);
  });

  casein::window_title = "poc-chunk";
}
