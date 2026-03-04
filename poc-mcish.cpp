#pragma leco app
#pragma leco add_resource_dir assets

import buffers;
import casein;
import chunk;
import dotz;
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

static enum {
  e_shadowtest,
  e_chunks,
} constexpr const example = e_chunks;

using namespace wagen;

static constexpr const auto target_fps = 30.f;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

class scene_drawer : public ofs::drawer {
  static constexpr const unsigned len = 32; // PoT padding
  static constexpr const unsigned count = len * len * len; // PoT padding
  static_assert(chunk::len <= len);

  texmap::cache m_tmap {};
  buffers::all m_bufs {
    count,
    models::corner::t {},
    models::cube::t {},
    models::prism::t {},
  };
  chunk::input m_in {
    count,
    models::corner::t {},
    models::cube::t {},
    models::prism::t {},
  };

  chunk::stamp m_ch { *m_in.inst, len };
  voo::single_cb m_cb {};
  chunk::gpunator m_cgpu {{
    .len = len,
    .in = &m_in,
    .out = &m_bufs,
  }};

  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    if (pl) vee::cmd_bind_descriptor_set(cb, pl, 0, m_tmap.dset());
    m_bufs.cmd_draw_vtx(cb);
  }
  void edges(vee::command_buffer cb) override {
    m_bufs.cmd_draw_edg(cb);
  }

public:
  scene_drawer();

  [[nodiscard]] auto texture(sv name) { return m_tmap.load(name); }
  [[nodiscard]] auto & ch() { return m_ch; }

  void update() {
    auto cb = m_cb.cb();
    {
      voo::cmd_buf_one_time_submit ots { cb };

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
          vee::memory_barrier(0, 0));

      vee::cmd_execute_command(cb, m_cgpu.command_buffer());

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
          vee::memory_barrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT));
    }

    voo::queue::universal()->submit({ .command_buffer = cb });
  }
};

static void ex_shadow_test(scene_drawer & embed) {
  static constexpr const sv t040 = "Tiles040_1K-JPG_Color.jpg";
  static constexpr const sv t101 = "Tiles101_1K-JPG_Color.jpg";
  static constexpr const sv t131 = "Tiles131_1K-JPG_Color.jpg";

  unsigned txt_ids[] {
    embed.texture(t040),
    embed.texture(t101),
    embed.texture(t131),
  };

  using enum chunk::model;
  auto & ch = embed.ch();

  // Prisms
  ch.set({ 4, 0, 5 }, { .mdl = prism, .txt = txt_ids[1] });

  // Cubes
  for (auto x = -chunk::minmax; x < chunk::minmax; x++) {
    for (auto y = -chunk::minmax; y < chunk::minmax; y++) {
      unsigned n = (x + y + 2 * chunk::minmax) % 4;
      if (n == 3) continue;
      ch.set({ x, -2, y }, { .mdl = cube, .txt = txt_ids[n] });
    }
  }
  ch.set({ 3, 0, 5 }, { .mdl = cube, .txt = txt_ids[0] });

  // More prisms
  ch.set({ 2, 0, 5 }, { .rot { 0, 1, 0, 0 }, .mdl = prism, .txt = txt_ids[1] });

  ch.copy({});
}

static void ex_chunks(scene_drawer & embed) {
  auto & ch = embed.ch();

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

  ch.copy({  0, 0, 0 });
  //ch.copy({  1, 0, 1 });
  //ch.copy({  0, 0, 1 });
  //ch.copy({ -1, 0, 1 });
  //ch.copy({  1, 0, 0 });
  //ch.copy({ -1, 0, 0 });
}

scene_drawer::scene_drawer() {
  switch (example) {
    case e_shadowtest: ex_shadow_test(*this); break;
    case e_chunks:     ex_chunks(*this);      break;
  }
  update();
}

struct app_stuff {
  voo::device_and_queue dq { "poc-model", casein::native_ptr, {
    .feats {
      .independentBlend = true,
      .samplerAnisotropy = true,
    },
  }};
  scene_drawer scene {};

  post::pipeline post { dq };
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

static float g_sun = 45;
static float g_sun_y = -1;

static float g_sun_spd = 90;
static sitime::stopwatch g_tt {};

static dotz::vec3 sun_vec() {
  dotz::vec3 l { dotz::sin(g_sun*3.14/180.), g_sun_y, dotz::cos(g_sun*3.14/180.) };
  return dotz::normalise(l);
}

static float g_far_plane = 15.f;
static constexpr const float far_plane_delta = 10.0f;
extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();
    auto & qp = vv::ss()->tq[vv::ss()->swc.index()];

    auto frame_secs = g_tt.secs();
    g_tt = {};

    g_sun += frame_secs * g_sun_spd;

    static float last = 0;
    {
      voo::cmd_buf_one_time_submit ots { cb };

      last = qp.write(cb, [&] {
        dotz::vec3 l = sun_vec();

        qp.write(timing::ppl_render, cb);
        vv::as()->ofs.render(cb, &vv::as()->scene, {
          .light { l, 0 },
          .aspect = vv::ss()->swc.aspect(),
          .far = g_far_plane,
        });

        qp.write(timing::ppl_post, cb);
        vv::as()->post.render(cb, vv::ss()->swc, {
          .fog { 0.4, 0.6, 0.8, 2 },
          .far = g_far_plane,
        });
      });
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();

    constexpr const auto frame_ms = 1000.0f / target_fps;
    if (g_far_plane < 15) g_far_plane++;
    if (last > frame_ms && g_far_plane > 15)  g_far_plane -= far_plane_delta * frame_secs;
    if (last < frame_ms && g_far_plane < 100) g_far_plane += far_plane_delta * frame_secs;

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

  casein::handle(casein::KEY_DOWN, casein::K_1, [] { g_sun_spd = -90.0; });
  casein::handle(casein::KEY_DOWN, casein::K_2, [] { g_sun_spd = -10.0; });
  casein::handle(casein::KEY_DOWN, casein::K_3, [] { g_sun_spd =   0.0; });
  casein::handle(casein::KEY_DOWN, casein::K_4, [] { g_sun_spd =  10.0; });
  casein::handle(casein::KEY_DOWN, casein::K_5, [] { g_sun_spd =  90.0; });

  casein::handle(casein::KEY_DOWN, casein::K_UP,   [] { g_sun_y = +1.0; });
  casein::handle(casein::KEY_DOWN, casein::K_DOWN, [] { g_sun_y = -1.0; });

  casein::handle(casein::KEY_DOWN, casein::K_LEFT,  [] { g_sun += 1.0; });
  casein::handle(casein::KEY_DOWN, casein::K_RIGHT, [] { g_sun -= 1.0; });

  casein::handle(casein::KEY_DOWN, casein::K_SPACE, [] {
    auto [x, y, z] = sun_vec();
    silog::infof("Sun vector: %-.3f, %-.3f, %-.3f", x, y, z);
    g_sun_spd = 0;
  });

  casein::window_title = "poc-mcish";
}
