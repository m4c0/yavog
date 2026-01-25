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

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

static constexpr const sv t040 = "Tiles040_1K-JPG_Color.jpg";
static constexpr const sv t101 = "Tiles101_1K-JPG_Color.jpg";
static constexpr const sv t131 = "Tiles131_1K-JPG_Color.jpg";

struct app_stuff : vinyl::base_app_stuff {
  cube::v_buffer cube {};
  clay::buffer<cube::inst> insts { 100 };
  cube::shadow_v_buffer shdvtx {};
  cube::shadow_ix_buffer shadows {};
  voo::bound_buffer idx = cube::ix_buffer();

  texmap::cache tmap {};
  hai::array<unsigned> txt_ids { 3 };

  ofs::pipeline ofs {};
  post::pipeline post { dq, false };

  app_stuff() : base_app_stuff { "poc-mcish" } {
    txt_ids[0] = tmap.load(t040);
    txt_ids[1] = tmap.load(t101);
    txt_ids[2] = tmap.load(t131);

    auto m = insts.map();
    m += cube::inst { .pos { 0, -1, 5 } };
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

static float g_sun = 45;
static float g_sun_y = 1;

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();

    {
      voo::cmd_buf_one_time_submit ots { cb };

      dotz::vec3 l { dotz::sin(g_sun*3.14/180.), g_sun_y, dotz::cos(g_sun*3.14/180.) };
      vv::as()->shadows.setup(l);
      vv::as()->ofs.render(cb, {
        .vtx = *vv::as()->cube,
        .inst = *vv::as()->insts,
        .idx = *vv::as()->idx.buffer,
        .shdvtx = *vv::as()->shdvtx,
        .shdidx = *vv::as()->shadows,
        .sicount = vv::as()->shadows.count(),
        .icount = vv::as()->insts.count(),
        .tmap = vv::as()->tmap.dset(),
        .light { l, 0 },
      });

      vv::as()->post.render(cb, vv::ss()->swc);
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::handle(casein::KEY_DOWN, casein::K_F, [] {
    casein::fullscreen = !casein::fullscreen;
    casein::interrupt(casein::IRQ_FULLSCREEN);
  });

  casein::handle(casein::KEY_DOWN, casein::K_LEFT,  [] { g_sun -= 4.0; });
  casein::handle(casein::KEY_DOWN, casein::K_RIGHT, [] { g_sun += 4.0; });

  casein::handle(casein::KEY_DOWN, casein::K_UP,   [] { g_sun_y = +1.0; });
  casein::handle(casein::KEY_DOWN, casein::K_DOWN, [] { g_sun_y = -1.0; });
}
