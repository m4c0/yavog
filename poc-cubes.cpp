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
  clay::buffer<ofs::inst> insts { 100 };
  cube::shadow_v_buffer shdvtx {};
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
    m += { .pos { -1,  0, 4 }, .txtid = 1 };
    m += { .pos {  1,  0, 4 }, .txtid = 1 };
    m += { .pos { -1,  0, 2 }, .txtid = 1 };
    m += { .pos {  1,  0, 2 }, .txtid = 1 };
    m += { .pos {  0, -1, 3 }, .txtid = 1 };
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
static float g_sun_y = -1;

static float g_sun_spd = 90;
static sitime::stopwatch g_tt {};

static dotz::vec3 sun_vec() {
  dotz::vec3 l { dotz::sin(g_sun*3.14/180.), g_sun_y, dotz::cos(g_sun*3.14/180.) };
  return dotz::normalise(l);
}

struct drawer : ofs::drawer {
  void faces(VkCommandBuffer cb, VkPipelineLayout pl) {
    if (pl) vee::cmd_bind_descriptor_set(cb, pl, 0, vv::as()->tmap.dset());
    vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->cube, 0);
    vee::cmd_bind_vertex_buffers(cb, 1, *vv::as()->insts, 0);
    vee::cmd_bind_index_buffer_u16(cb, *vv::as()->idx.buffer);
    vee::cmd_draw_indexed(cb, {
      .xcount = 36,
      .icount = vv::as()->insts.count(),
    });
  }
  void edges(VkCommandBuffer cb) {
    vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->shdvtx, 0);
    vee::cmd_draw(cb, {
      .vcount = 36,
      .icount = vv::as()->insts.count(),
    });
  }
};

static constexpr const float far_plane = 100.f;
extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();

    g_sun += g_tt.secs() * g_sun_spd;
    g_tt = {};

    {
      voo::cmd_buf_one_time_submit ots { cb };

      drawer d {};

      dotz::vec3 l = sun_vec();
      vv::as()->ofs.render(cb, &d, {
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
