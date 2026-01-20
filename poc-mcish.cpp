#pragma leco app
#pragma leco add_resource_dir assets

import casein;
import clay;
import cube;
import dotz;
import hai;
import ofs;
import post;
import shadowmap;
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
  cube::i_buffer insts {};
  cube::shadow_ix_buffer shadows {};
  voo::bound_buffer idx = cube::ix_buffer();

  texmap::cache tmap {};
  hai::array<unsigned> txt_ids { 3 };

  post::pipeline post { dq };
  shadowmap::pipeline shadow {};
  ofs::pipeline ofs {};

  app_stuff() : base_app_stuff { "poc-mcish" } {
    txt_ids[0] = tmap.load(t040);
    txt_ids[1] = tmap.load(t101);
    txt_ids[2] = tmap.load(t131);

    ofs.update_descriptor_sets(shadow.iv());

    shadows.setup({ 0, -1, 1 });
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

static float g_sun = 180;
static float sun_angle() { return g_sun; }
static void render_scene(vee::command_buffer cb) {
  vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->cube, 0);
  vee::cmd_bind_vertex_buffers(cb, 1, *vv::as()->insts, 0);
  vee::cmd_bind_index_buffer_u16(cb, *vv::as()->idx.buffer);
  vee::cmd_draw_indexed(cb, {
    .xcount = 36,
    .icount = vv::as()->insts.count(),
  });

  vee::cmd_bind_index_buffer_u16(cb, *vv::as()->shadows);
  vee::cmd_draw_indexed(cb, {
    .xcount = vv::as()->shadows.count(),
    .icount = vv::as()->insts.count(),
  });
}
static void render_to_shadowmap(vee::command_buffer cb) {
  auto rpg = vv::as()->shadow.cmd_render_pass(cb, sun_angle());
  render_scene(cb);
}
static void render_to_offscreen(vee::command_buffer cb) {
  auto rpg = vv::as()->ofs.cmd_render_pass(cb, vv::as()->tmap.dset(), sun_angle());
  render_scene(cb);
}

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();
    auto & qp = vv::ss()->tq[vv::ss()->swc.index()];

    {
      voo::cmd_buf_one_time_submit ots { cb };

      qp.write_begin(cb);

      render_to_shadowmap(cb);

      qp.write_postshadow(cb);

      render_to_offscreen(cb);

      qp.write_prepost(cb);

      vv::as()->post.render(cb, vv::ss()->swc);

      qp.write_end(cb);
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();

    static struct count {
      sitime::stopwatch w {};
      int i = 0;
      ~count() { silog::infof(">>>> %.3f", i / w.secs()); }
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

  casein::handle(casein::KEY_DOWN, casein::K_LEFT,  [] { g_sun -= 1.0; });
  casein::handle(casein::KEY_DOWN, casein::K_RIGHT, [] { g_sun += 1.0; });
}
