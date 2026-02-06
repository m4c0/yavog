#pragma leco app
#pragma leco add_resource_dir assets

import casein;
import clay;
import cube;
import dotz;
import hai;
import ofs;
import prism;
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

//#define CUBE_EXAMPLE

using namespace wagen;

static constexpr const auto target_fps = 30.f;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

class scene_drawer : public ofs::drawer {
  texmap::cache tmap {};
  ofs::v_buffer  vtx { cube::t {}, prism::t {} };
  ofs::ix_buffer idx { cube::t {}, prism::t {} };
  ofs::e_buffer  edg { cube::t {}, prism::t {} };

  ofs::buffer<ofs::inst> ins { 128 * 128 * 2 };
  ofs::buffer<VkDrawIndexedIndirectCommand> vtx_cmd { 2 };
  ofs::buffer<VkDrawIndirectCommand> edg_cmd { 2 };

public:
  scene_drawer();

  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    if (pl) vee::cmd_bind_descriptor_set(cb, pl, 0, tmap.dset());

    vee::cmd_bind_vertex_buffers(cb, 0, *vtx, 0);
    vee::cmd_bind_vertex_buffers(cb, 1, *ins, 0);
    vee::cmd_bind_index_buffer_u16(cb, *idx);
    for (auto i = 0; i < vtx_cmd.count(); i++) {
      vee::cmd_draw_indexed_indirect(cb, *vtx_cmd, i, 1);
    }
  }
  void edges(vee::command_buffer cb) override {
    vee::cmd_bind_vertex_buffers(cb, 0, *edg, 0);
    vee::cmd_bind_vertex_buffers(cb, 1, *ins, 0);
    for (auto i = 0; i < edg_cmd.count(); i++) {
      vee::cmd_draw_indirect(cb, *edg_cmd, i, 1);
    }
  }
};

#ifndef CUBE_EXAMPLE
scene_drawer::scene_drawer() {
  static constexpr const sv t040 = "Tiles040_1K-JPG_Color.jpg";
  static constexpr const sv t101 = "Tiles101_1K-JPG_Color.jpg";
  static constexpr const sv t131 = "Tiles131_1K-JPG_Color.jpg";

  unsigned txt_ids[] {
    tmap.load(t040),
    tmap.load(t101),
    tmap.load(t131),
  };

  enum {
    m_cube,
    m_prism,
  };
  struct {
    unsigned i_count;
    unsigned first_i;
    int v_offset;
    unsigned v_count;
    unsigned first_v;
  } mdls[] {
    {
      .i_count = ofs::size(cube::t::tri) * 3,
      .first_i = 0,
      .v_offset = 0,
      .v_count = ofs::size(cube::t::edg) * 3,
      .first_v = 0,
    },
    {
      .i_count = ofs::size(prism::t::tri) * 3,
      .first_i = ofs::size(cube::t::tri) * 3,
      .v_offset = ofs::size(cube::t::vtx),
      .v_count = ofs::size(prism::t::edg) * 3,
      .first_v = ofs::size(cube::t::edg) * 3,
    },
  };

  auto vc = vtx_cmd.map();
  auto ec = edg_cmd.map();

  auto m = ins.map();
  // Prisms
  m += {
    .pos { 4, 0, 5 },
    .txtid = static_cast<float>(txt_ids[1]),
  };
  m += {
    .pos { 2, 0, 5 },
    .txtid = static_cast<float>(txt_ids[1]),
    .rot { 0, 1, 0, 0 },
  };
  vc += {
    .indexCount = mdls[m_prism].i_count,
    .instanceCount = m.count(),
    .firstIndex = mdls[m_prism].first_i,
    .vertexOffset = mdls[m_prism].v_offset,
    .firstInstance = 0,
  };
  ec += {
    .vertexCount = mdls[m_prism].v_count,
    .instanceCount = m.count(),
    .firstVertex = mdls[m_prism].first_v,
    .firstInstance = 0,
  };
  auto p_count = m.count();

  // Cubes
  for (auto x = 0; x < 128; x++) {
    for (auto y = 0; y < 128; y++) {
      unsigned n = (x + y) % 4;
      if (n == 3) continue;

      m += {
        .pos { x - 64, -2, y },
        .txtid = static_cast<float>(txt_ids[n]),
      };
    }
  }
  m += {
    .pos { 3, 0, 5 },
    .txtid = static_cast<float>(txt_ids[0]),
  };
  vc += {
    .indexCount = mdls[m_cube].i_count,
    .instanceCount = m.count() - p_count,
    .firstIndex = mdls[m_cube].first_i,
    .vertexOffset = mdls[m_cube].v_offset,
    .firstInstance = p_count,
  };
  ec += {
    .vertexCount = mdls[m_cube].v_count,
    .instanceCount = m.count() - p_count,
    .firstVertex = mdls[m_cube].first_v,
    .firstInstance = p_count,
  };
  p_count = m.count();
}
#else
scene_drawer::scene_drawer() {
  float txt_id = tmap.load("Tiles101_1K-JPG_Color.jpg");

  auto m = cube.map();
  m += { .pos { -1,  0, 4 }, .txtid = txt_id };
  m += { .pos {  1,  0, 4 }, .txtid = txt_id };
  m += { .pos { -1,  0, 2 }, .txtid = txt_id };
  m += { .pos {  1,  0, 2 }, .txtid = txt_id };
  m += { .pos {  0, -1, 3 }, .txtid = txt_id };
}
#endif

struct app_stuff {
  voo::device_and_queue dq { "poc-model", casein::native_ptr, {
    .feats {
      .independentBlend = true,
      .samplerAnisotropy = true,
    },
  }};
  scene_drawer scene {};

#ifndef CUBE_EXAMPLE
  post::pipeline post { dq };
#else
  post::pipeline post { dq, false };
#endif
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
#ifndef CUBE_EXAMPLE
        vv::as()->post.render(cb, vv::ss()->swc, {
          .fog { 0.4, 0.6, 0.8, 2 },
          .far = g_far_plane,
        });
#else
        vv::as()->post.render(cb, vv::ss()->swc, {});
#endif
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

  casein::window_title = "poc-model";
}
