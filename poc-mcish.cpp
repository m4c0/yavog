#pragma leco app
#pragma leco add_resource_dir assets

import buffers;
import casein;
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

struct model_cmd {
  unsigned id;
  unsigned i_count;
  unsigned first_i;
  int v_offset;
  unsigned v_count;
  unsigned e_count;
  unsigned first_e;

  template<typename T> static model_cmd of(T) {
    return {
      .id = T::id,

      .i_count = buffers::size(T::tri) * 3,
      .v_count = buffers::size(T::vtx),
      .e_count = buffers::size(T::edg) * 3,
    };
  }
};
class model_cmds {
  static constexpr const auto max = 16;
  buffers::buffer<VkDrawIndexedIndirectCommand> m_vtx { max };
  buffers::buffer<VkDrawIndirectCommand> m_edg { max };

  voo::memiter<VkDrawIndexedIndirectCommand> m_vc = m_vtx.map();
  voo::memiter<VkDrawIndirectCommand> m_ec = m_edg.map();

  hai::varray<model_cmd> m_mdls;
  model_cmd m_last {};
  unsigned m_count {};

  template<typename T> void cmd(T) {
    auto m = model_cmd::of(T {});

    m.first_i  = m_last.first_i  + m_last.i_count;
    m.v_offset = m_last.v_offset + m_last.v_count;
    m.first_e  = m_last.first_e  + m_last.e_count;

    m_last = m;
    m_mdls.push_back(m);
  }

public:
  template<typename... T>
  model_cmds(T...) : m_mdls { sizeof...(T) } {
    (cmd(T {}), ...);
  }

  template<typename T>
  [[nodiscard]] constexpr model_cmd operator[](T) const {
    for (auto m : m_mdls) if (m.id == T::id) return m;
    return {};
  }

  void push(model_cmd m, unsigned count) {
    m_vc += {
      .indexCount = m.i_count,
      .instanceCount = count - m_count,
      .firstIndex = m.first_i,
      .vertexOffset = m.v_offset,
      .firstInstance = m_count,
    };
    m_ec += {
      .vertexCount = m.e_count,
      .instanceCount = count - m_count,
      .firstVertex = m.first_e,
      .firstInstance = m_count,
    };

    m_count = count;
  }

  void faces(vee::command_buffer cb) const {
    for (auto i = 0; i < m_vtx.count(); i++) {
      vee::cmd_draw_indexed_indirect(cb, *m_vtx, i, 1);
    }
  }
  void edges(vee::command_buffer cb) const {
    for (auto i = 0; i < m_edg.count(); i++) {
      vee::cmd_draw_indirect(cb, *m_edg, i, 1);
    }
  }
};

class scene_drawer : public ofs::drawer {
  texmap::cache tmap {};
  buffers::v_buffer  vtx { cube::t {}, prism::t {} };
  buffers::ix_buffer idx { cube::t {}, prism::t {} };
  buffers::e_buffer  edg { cube::t {}, prism::t {} };
  model_cmds        mdls { cube::t {}, prism::t {} };

  buffers::buffer<buffers::inst> ins { 128 * 128 * 2 };

public:
  scene_drawer();

  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    if (pl) vee::cmd_bind_descriptor_set(cb, pl, 0, tmap.dset());

    vee::cmd_bind_vertex_buffers(cb, 0, *vtx, 0);
    vee::cmd_bind_vertex_buffers(cb, 1, *ins, 0);
    vee::cmd_bind_index_buffer_u16(cb, *idx);
    mdls.faces(cb);
  }
  void edges(vee::command_buffer cb) override {
    vee::cmd_bind_vertex_buffers(cb, 0, *edg, 0);
    vee::cmd_bind_vertex_buffers(cb, 1, *ins, 0);
    mdls.edges(cb);
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

  auto m = ins.map();
  // Prisms
  m += {
    .pos { 4, 0, 5, static_cast<float>(txt_ids[1]) },
  };
  m += {
    .pos { 2, 0, 5, static_cast<float>(txt_ids[1]) },
    .rot { 0, 1, 0, 0 },
  };
  mdls.push(mdls[prism::t()], m.count());

  // Cubes
  for (auto x = 0; x < 128; x++) {
    for (auto y = 0; y < 128; y++) {
      unsigned n = (x + y) % 4;
      if (n == 3) continue;

      m += {
        .pos { x - 64, -2, y, static_cast<float>(txt_ids[n]) },
      };
    }
  }
  m += {
    .pos { 3, 0, 5, static_cast<float>(txt_ids[0]) },
  };
  mdls.push(mdls[cube::t()], m.count());
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
