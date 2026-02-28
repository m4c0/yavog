#pragma leco app
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
  voo::device_and_queue dq { "poc-chunk", casein::native_ptr, {
    .feats {
      .independentBlend = true,
      .samplerAnisotropy = true,
    },
  }};
  vee::render_pass rp = voo::single_att_render_pass(dq);

  post::pipeline post { dq };
  ofs::pipeline ofs {};

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
      ch.copy(m, { 0, 0, 0 }, len);
    }

    {
      auto cb = scb.cb();

      voo::cmd_buf_one_time_submit ots { cb };

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
          vee::memory_barrier(0, 0));

      vee::cmd_execute_command(cb, cgpu.command_buffer());

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
          vee::memory_barrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT));
    }

    voo::queue::universal()->submit({ .command_buffer = scb.cb() });

    indirectdebug::dump(bufs, count, 4);
  }
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };

  ext_stuff() {
    vv::as()->ofs.setup(swc);

    vv::as()->post.update_descriptor_sets(vv::as()->ofs);
    vv::as()->post.setup(swc);
  }
};

struct scene_drawer : public ofs::drawer {
  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    if (pl) vee::cmd_bind_descriptor_set(cb, pl, 0, vv::as()->tmap.dset());
    vv::as()->bufs.cmd_draw_vtx(cb);
  }
  void edges(vee::command_buffer cb) override {
    vv::as()->bufs.cmd_draw_edg(cb);
  }
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();

    auto l = dotz::normalise(dotz::vec3 { 1, -0.3, 0.3 });
    scene_drawer d {};

    auto cb = vv::ss()->cb.cb();
    {
      voo::cmd_buf_one_time_submit ots { cb };
      vv::as()->ofs.render(cb, &d, {
        .light { l, 0 },
        .aspect = vv::ss()->swc.aspect(),
        .far = 100.0f,
      });
      vv::as()->post.render(cb, vv::ss()->swc, {
        .fog { 0.4, 0.6, 0.8, 2 },
        .far = 100.f,
      });
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
