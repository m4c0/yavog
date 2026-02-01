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

class scene_drawer : public ofs::drawer {
  texmap::cache tmap {};

  cube::drawer cube {};

public:
  scene_drawer() {
    float txt_id = tmap.load("Tiles101_1K-JPG_Color.jpg");

    auto m = cube.map();
    m += { .pos { -1,  0, 4 }, .txtid = txt_id };
    m += { .pos {  1,  0, 4 }, .txtid = txt_id };
    m += { .pos { -1,  0, 2 }, .txtid = txt_id };
    m += { .pos {  1,  0, 2 }, .txtid = txt_id };
    m += { .pos {  0, -1, 3 }, .txtid = txt_id };
  }

  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    if (pl) vee::cmd_bind_descriptor_set(cb, pl, 0, tmap.dset());
    cube.faces(cb, pl);
  }
  void edges(vee::command_buffer cb) override {
    cube.edges(cb);
  }
};

struct app_stuff : vinyl::base_app_stuff {
  scene_drawer scene {};

  ofs::pipeline ofs {};
  post::pipeline post { dq, false };

  app_stuff() : base_app_stuff { "poc-mcish" } {}
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
static float g_sun_y = -1;

static float g_sun_spd = 90;
static sitime::stopwatch g_tt {};

static dotz::vec3 sun_vec() {
  dotz::vec3 l { dotz::sin(g_sun*3.14/180.), g_sun_y, dotz::cos(g_sun*3.14/180.) };
  return dotz::normalise(l);
}

static constexpr const float far_plane = 100.f;
extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();

    g_sun += g_tt.secs() * g_sun_spd;
    g_tt = {};

    {
      voo::cmd_buf_one_time_submit ots { cb };

      dotz::vec3 l = sun_vec();
      vv::as()->ofs.render(cb, &vv::as()->scene, {
        .light { l, 0 },
        .aspect = vv::ss()->swc.aspect(),
        .far = far_plane,
      });

      vv::as()->post.render(cb, vv::ss()->swc, {});
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
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
}
