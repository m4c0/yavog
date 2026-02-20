#pragma leco app
#pragma leco add_resource_dir assets

import buffers;
import casein;
import chunk;
import dotz;
import embedded;
import hai;
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

class scene_drawer : public ofs::drawer {
  models::drawer embed { 128 * 128 * 16 };
  chunk::t ch {};

  voo::single_cb scb {};

  static_assert(chunk::len <= 32);
  static constexpr const unsigned count = 32 * 32 * 32; // PoT padding
  buffers::buffer<buffers::tmp_inst> host { count };
  voo::bound_buffer local0 = voo::bound_buffer::create_from_host(sizeof(buffers::inst) * count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  voo::bound_buffer local1 = voo::bound_buffer::create_from_host(sizeof(buffers::inst) * count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

  chunk::compact ccomp { *host, *local0.buffer, *local1.buffer };

public:
  scene_drawer();

  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    embed.faces(cb, pl);
  }
  void edges(vee::command_buffer cb) override {
    embed.edges(cb);
  }

  void build(float mult = 1) {
    {
      auto m = host.map();
      ch.copy(m, { 0, 0, 32 }, mult);
    }

    using enum chunk::model;
    auto m = embed.builder();
    ch.build(m, cube, { 0, 0, 32 }, mult);
    m.push(embed.model(models::cube::t {}));
    ch.build(m, prism, { 0, 0, 32 }, mult);
    m.push(embed.model(models::prism::t {}));
    ch.build(m, corner, { 0, 0, 32 }, mult);
    m.push(embed.model(models::corner::t {}));

    bool use_0 = true;
    auto cb = scb.cb();
    {
      voo::cmd_buf_one_time_submit ots { cb };

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
          vee::memory_barrier(0, 0));

      use_0 = ccomp.cmd(cb, 32);

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
          vee::memory_barrier(0, 0));
    }
    voo::queue::universal()->submit({ .command_buffer = cb });
  }
};

scene_drawer::scene_drawer() {
  using enum chunk::model;
  auto dirt  = embed.texture("Ground105_1K-JPG_Color.jpg");
  auto grass = embed.texture("Ground037_1K-JPG_Color.jpg");

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

  build();
}

struct app_stuff {
  voo::device_and_queue dq { "poc-model", casein::native_ptr, {
    .feats {
      .independentBlend = true,
      .samplerAnisotropy = true,
    },
  }};
  scene_drawer scene {};

  post::pipeline post { dq, false };
  ofs::pipeline ofs {};
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };

  hai::array<timing::query> tq { swc.count() };

  ext_stuff() {
    vv::as()->ofs.setup(swc);

    vv::as()->post.update_descriptor_sets(vv::as()->ofs);
    vv::as()->post.setup(swc);
  }
};

static const float g_far_plane = 100.f;
extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();
    auto & qp = vv::ss()->tq[vv::ss()->swc.index()];

    static float last = 0;
    {
      voo::cmd_buf_one_time_submit ots { cb };

      last = qp.write(cb, [&] {
        qp.write(timing::ppl_render, cb);
        vv::as()->ofs.render(cb, &vv::as()->scene, {
          .light { 0, -0.71, 0.71, 0 },
          .aspect = vv::ss()->swc.aspect(),
          .far = g_far_plane,
        });

        qp.write(timing::ppl_post, cb);
        vv::as()->post.render(cb, vv::ss()->swc, {});
      });
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();

    static struct count {
      sitime::stopwatch w {};
      int i = 0;
      ~count() { silog::infof("Counters: %.3f FPS, last frame: %.1fms, far plane %.0f", i / w.secs(), last, g_far_plane); }
    } * cnt = new count {};
    cnt->i++;
    if (cnt->w.secs() > 3) {
      delete cnt;
      cnt = new count {};
    }
  });

  casein::handle(casein::KEY_DOWN, casein::K_F, [] {
    casein::fullscreen = !casein::fullscreen;
    casein::interrupt(casein::IRQ_FULLSCREEN);
  });

  casein::handle(casein::KEY_DOWN, casein::K_UP,   [] { });
  casein::handle(casein::KEY_DOWN, casein::K_DOWN, [] { });

  casein::handle(casein::KEY_DOWN, casein::K_LEFT,  [] { });
  casein::handle(casein::KEY_DOWN, casein::K_RIGHT, [] { });

  casein::handle(casein::KEY_DOWN, casein::K_SPACE, [] {
    static bool explode = true;
    vv::as()->scene.build(explode ? 1.5 : 1.0);
    explode = !explode;
  });

  casein::window_title = "poc-chunk";
}
