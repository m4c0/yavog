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

using namespace wagen;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

class scene_drawer : public ofs::drawer {
  static constexpr const dotz::ivec3 len { 256, 32, 256 };
  static constexpr const unsigned count = len.z * len.y * len.x;
  static_assert(chunk::len <= len.x);
  static_assert(chunk::len <= len.y);
  static_assert(chunk::len <= len.z);
  static_assert((len.x & (len.x - 1)) == 0, "len must be power of two");
  static_assert((len.y & (len.y - 1)) == 0, "len must be power of two");
  static_assert((len.z & (len.z - 1)) == 0, "len must be power of two");

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

      vee::cmd_execute_command(cb, m_cgpu.command_buffer());

      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
          vee::memory_barrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT));
    }

    voo::queue::universal()->submit({ .command_buffer = cb });
  }
};

scene_drawer::scene_drawer() {
  static constexpr const sv t040 = "Tiles040_1K-JPG_Color.jpg";
  static constexpr const sv t101 = "Tiles101_1K-JPG_Color.jpg";
  static constexpr const sv t131 = "Tiles131_1K-JPG_Color.jpg";

  unsigned txt_ids[] {
    texture(t040),
    texture(t101),
    texture(t131),
  };

  using enum chunk::model;
  auto & ch = this->ch();

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

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();
    auto & qp = vv::ss()->tq[vv::ss()->swc.index()];

    auto frame_secs = g_tt.secs();
    g_tt = {};

    g_sun += frame_secs * g_sun_spd;

    {
      voo::cmd_buf_one_time_submit ots { cb };

      qp.write(cb, [&] {
        dotz::vec3 l = sun_vec();

        qp.write(timing::ppl_render, cb);
        vv::as()->ofs.render(cb, &vv::as()->scene, {
          .light { l, 0 },
          .aspect = vv::ss()->swc.aspect(),
          .far = 32,
        });

        qp.write(timing::ppl_post, cb);
        vv::as()->post.render(cb, vv::ss()->swc, {
          .fog { 0.4, 0.6, 0.8, 2 },
          .far = 32,
        });
      });
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::handle(casein::KEY_DOWN, casein::K_F, [] {
    casein::fullscreen = !casein::fullscreen;
    casein::interrupt(casein::IRQ_FULLSCREEN);
  });

  casein::handle(casein::KEY_DOWN, casein::K_UP,   [] { g_sun_y = -0.3; });
  casein::handle(casein::KEY_DOWN, casein::K_DOWN, [] { g_sun_y = -1.0; });

  casein::window_title = "poc-shadowtest";
}
