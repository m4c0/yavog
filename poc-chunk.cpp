#pragma leco app
#pragma leco add_shader "chunk.comp"
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

struct inst {
  dotz::vec4 rot;
  dotz::vec3 pos;
  float mdl;
  dotz::vec3 size;
  float txt;
};

class scene_drawer : public ofs::drawer {
  models::drawer embed { 128 * 128 * 16 };
  chunk::t ch {};

  vee::descriptor_set_layout dsl = vee::create_descriptor_set_layout({
    vee::dsl_compute_storage(),
    vee::dsl_compute_storage(),
  });
  vee::descriptor_pool dpool = vee::create_descriptor_pool(2, {
    vee::storage_buffer(4),
  });
  vee::descriptor_set dset0 = vee::allocate_descriptor_set(*dpool, *dsl);
  vee::descriptor_set dset1 = vee::allocate_descriptor_set(*dpool, *dsl);

  vee::pipeline_layout pl = vee::create_pipeline_layout(*dsl, vee::compute_push_constant_range<dotz::ivec3>());
  vee::c_pipeline ppl = vee::create_compute_pipeline(*pl, *voo::comp_shader("chunk.comp.spv"), "main");
  voo::single_cb scb {};

  static constexpr const unsigned count = chunk::len * chunk::len * chunk::len;
  buffers::buffer<buffers::tmp_inst> host { count };
  voo::bound_buffer local = voo::bound_buffer::create_from_host(sizeof(inst) * count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

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

    auto cb = scb.cb();
    {
      constexpr const dotz::ivec3 ivec3_x { 1, 0, 0 };
      constexpr const dotz::ivec3 ivec3_y { 0, 1, 0 };
      constexpr const dotz::ivec3 ivec3_z { 0, 0, 1 };

      voo::cmd_buf_one_time_submit ots { cb };

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
          vee::buffer_memory_barrier(*host, VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT));

      vee::cmd_bind_c_pipeline(cb, *ppl);
      vee::cmd_bind_c_descriptor_set(cb, *pl, 0, dset0);
      vee::cmd_push_compute_constants(cb, *pl, &ivec3_z);
      vee::cmd_dispatch(cb, chunk::len, chunk::len, 1);

      vee::cmd_bind_c_descriptor_set(cb, *pl, 0, dset1);
      vee::cmd_push_compute_constants(cb, *pl, &ivec3_y);
      vee::cmd_dispatch(cb, chunk::len, 1, chunk::len);

      vee::cmd_push_compute_constants(cb, *pl, &ivec3_x);
      vee::cmd_dispatch(cb, 1, chunk::len, chunk::len);

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
          vee::buffer_memory_barrier(*local.buffer, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT));
    }
    voo::queue::universal()->submit({ .command_buffer = cb });
  }
};

scene_drawer::scene_drawer() {
  using enum chunk::model;
  auto dirt  = embed.texture("Ground105_1K-JPG_Color.jpg");
  auto grass = embed.texture("Ground037_1K-JPG_Color.jpg");

  vee::update_descriptor_set(dset0, 0, *host);
  vee::update_descriptor_set(dset0, 1, *local.buffer);

  vee::update_descriptor_set(dset1, 0, *local.buffer);
  vee::update_descriptor_set(dset1, 1, *local.buffer);

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
