#pragma leco app
#pragma leco add_resource_dir assets

import buffers;
import casein;
import chunk;
import dotz;
import hai;
import models;
import msaa;
import ofs;
import poc;
import post;
import silog;
import sitime;
import skybox;
import sv;
import texmap;
import timing;
import traits;
import vinyl;
import voo;
import wagen;

using namespace wagen;

static constexpr const auto target_fps = 30.f;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

class scene_drawer : public buffers::vk::drawer {
  static constexpr const dotz::ivec3 len { 256, 32, 256 };
  static_assert(chunk::len <= len.x);
  static_assert(chunk::len <= len.y);
  static_assert(chunk::len <= len.z);
  static_assert((len.x & (len.x - 1)) == 0, "len must be power of two");
  static_assert((len.y & (len.y - 1)) == 0, "len must be power of two");
  static_assert((len.z & (len.z - 1)) == 0, "len must be power of two");

  texmap::cache m_tmap {};

  chunk::gpunator m_cgpu {
    len,
    models::corner::t {},
    models::cube::t {},
    models::prism::t {},
  };
  chunk::stamp m_ch = m_cgpu.stamp();

  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    if (pl) vee::cmd_bind_descriptor_set(cb, pl, 0, m_tmap.dset());
    m_cgpu.cmd_draw_vtx(cb);
  }
  void edges(vee::command_buffer cb) override {
    m_cgpu.cmd_draw_edg(cb);
  }

public:
  scene_drawer();
};

scene_drawer::scene_drawer() {
  auto & ch = m_ch;

  using enum chunk::model;
  auto dirt  = m_tmap.load("Ground105_1K-JPG_Color.jpg");
  auto grass = m_tmap.load("Ground037_1K-JPG_Color.jpg");

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
  ch.copy({  1, 0, 1 });
  ch.copy({  0, 0, 1 });
  ch.copy({ -1, 0, 1 });
  ch.copy({  1, 0, 0 });
  ch.copy({ -1, 0, 0 });

  m_cgpu.submit();
}

struct app_stuff {
  voo::device_and_queue dq = poc::device_and_queue("poc-mcish");
  scene_drawer scene {};

  post::pipeline post { dq };
  ofs::pipeline ofs {};
  skybox::pipeline sky {};
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };
  msaa::framebuffer msaa { swc.extent() };

  hai::array<timing::query> tq { swc.count() };

  ext_stuff() {
    vv::as()->post.update_descriptor_sets(msaa);
    vv::as()->post.setup(swc);
  }
};

static float g_sun = 45;
static float g_sun_y = -1;

static float g_sun_spd = 90;
static sitime::stopwatch g_tt {};

static dotz::vec3 sun_vec() {
  if (g_sun_y < -2) g_sun_y = -2;
  else if (g_sun_y > -0.1) g_sun_y = -0.1;

  dotz::vec3 l { dotz::sin(g_sun*3.14/180.), g_sun_y, dotz::cos(g_sun*3.14/180.) };
  return dotz::normalise(l);
}

static float g_far_plane = 15.f;
static constexpr const float far_plane_delta = 10.0f;

static float g_sky_far_plane = 0.f;
static void update_skybox() {
  vv::as()->sky.render_to_cubemap(&vv::as()->scene, {
    .far = 128.0,
    .near = g_far_plane,
  });
  g_sky_far_plane = g_far_plane;
}

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();

    if (dotz::abs(g_far_plane - g_sky_far_plane) > 6) update_skybox();

    auto frame_secs = g_tt.secs();
    g_tt = {};

    g_sun += frame_secs * g_sun_spd;

    static float last = 0;
    {
      voo::cmd_buf_one_time_submit ots { cb };

      vv::ss()->msaa.cmd_render_pass(cb, g_far_plane, [&] {
        dotz::vec3 l = sun_vec();

        vv::as()->ofs.render(cb, &vv::as()->scene, {
          .light { l, 0 },
          .aspect = vv::ss()->swc.aspect(),
          .far = g_far_plane,
        });
        vv::as()->sky.cmd_draw(cb, vv::ss()->swc.aspect());
      });
      vv::as()->post.render(cb, vv::ss()->swc, {
        .fog { 0.4, 0.6, 0.8, 2 },
        .far = g_far_plane,
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

  casein::handle(casein::KEY_DOWN, casein::K_LEFT,  [] { g_sun += 0.1; });
  casein::handle(casein::KEY_DOWN, casein::K_RIGHT, [] { g_sun -= 0.1; });

  casein::handle(casein::KEY_DOWN, casein::K_SPACE, [] {
    auto [x, y, z] = sun_vec();
    silog::infof("Sun vector: %-.3f, %-.3f, %-.3f", x, y, z);
    g_sun_spd = 0;
  });

  casein::window_title = "poc-mcish";
}
