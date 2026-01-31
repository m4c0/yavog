#pragma leco app
#pragma leco add_resource_dir assets

import casein;
import clay;
import cube;
import dotz;
import hai;
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

static constexpr const auto target_fps = 30.f;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

static constexpr const sv t040 = "Tiles040_1K-JPG_Color.jpg";
static constexpr const sv t101 = "Tiles101_1K-JPG_Color.jpg";
static constexpr const sv t131 = "Tiles131_1K-JPG_Color.jpg";

struct app_stuff : vinyl::base_app_stuff {
  cube::v_buffer cube {};
  cube::i_buffer insts {};
  cube::shadow_v_buffer shdvtx {};
  voo::bound_buffer idx = cube::ix_buffer();

  texmap::cache tmap {};
  hai::array<unsigned> txt_ids { 3 };

  post::pipeline post { dq };
  ofs::pipeline ofs {};

  app_stuff() : base_app_stuff { "poc-mcish" } {
    txt_ids[0] = tmap.load(t040);
    txt_ids[1] = tmap.load(t101);
    txt_ids[2] = tmap.load(t131);
  }
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

static float g_far_plane = 0.f;
extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();
    auto & qp = vv::ss()->tq[vv::ss()->swc.index()];

    g_sun += g_tt.secs() * g_sun_spd;
    g_tt = {};

    static float last = 0;
    {
      voo::cmd_buf_one_time_submit ots { cb };

      last = qp.write(cb, [&] {
        dotz::vec3 l = sun_vec();

        qp.write(timing::ppl_render, cb);
        vv::as()->ofs.render(cb, {
          .vtx = *vv::as()->cube,
          .inst = *vv::as()->insts,
          .idx = *vv::as()->idx.buffer,
          .shdvtx = *vv::as()->shdvtx,
          .icount = vv::as()->insts.count(),
          .tmap = vv::as()->tmap.dset(),
          .light { l, 0 },
          .far = g_far_plane,
        });

        qp.write(timing::ppl_post, cb);
        vv::as()->post.render(cb, vv::ss()->swc, {
          .fog { 0.4, 0.6, 0.8, g_far_plane * 0.8 },
          .far = g_far_plane,
        });
      });
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();

    constexpr const auto frame_ms = 1000.0f / target_fps;
    if (g_far_plane < 15) g_far_plane++;
    if (last > frame_ms && g_far_plane > 15) g_far_plane--;
    if (last < frame_ms && g_far_plane < 100) g_far_plane++;

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
}
