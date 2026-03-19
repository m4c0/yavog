#pragma leco app
#pragma leco add_resource_dir assets

import buffers;
import casein;
import chunk;
import dotz;
import models;
import msaa;
import ofs;
import poc;
import post;
import skybox;
import texmap;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

class scene_drawer : public buffers::vk::drawer {
  texmap::cache m_tmap {};
  chunk::gpunator m_cgpu {
    { 128, 32, 128 },
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
  scene_drawer() {
    using enum chunk::model;
    auto dirt  = m_tmap.load("Ground105_1K-JPG_Color.jpg");
    auto grass = m_tmap.load("Ground037_1K-JPG_Color.jpg");

    constexpr const auto mm = chunk::minmax;
    for (auto x = -mm; x <= mm; x++) {
      for (auto y = -mm; y <= -2; y++) {
        for (auto z = -mm; z <= mm; z++) {
          auto txt = (z == -mm || z == mm || x == -mm || x == mm) ? dirt : grass;
          m_ch.set({ x, -2, z }, { .mdl = cube, .txt = txt });
        }
      }
    }

    for (auto x = -4; x <= 4; x++) {
      for (auto z = -4; z <= 4; z++) {
        m_ch.copy({ x, 0, z });
      }
    }
    m_cgpu.submit();
  }
};

struct app_stuff {
  voo::device_and_queue dq = poc::device_and_queue("poc-chunk-skybox");
  scene_drawer scene {};

  post::pipeline post { dq, true };
  skybox::pipeline sky {};
  ofs::pipeline ofs {};
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };
  msaa::framebuffer msaa { swc.extent() };

  ext_stuff() {
    vv::as()->post.update_descriptor_sets(msaa);
    vv::as()->post.setup(swc);

    vv::as()->sky.render_to_cubemap(&vv::as()->scene, {
      .far = 100.0,
      .near = 26.0,
    });
  }
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();

    {
      voo::cmd_buf_one_time_submit ots { cb };

      vv::ss()->msaa.cmd_render_pass(cb, 36, [&] {
        vv::as()->ofs.render(cb, &vv::as()->scene, {
          .light { dotz::normalise(dotz::vec3 { -1 }), 0 },
          .aspect = vv::ss()->swc.aspect(),
          .far = 36,
        });
        vv::as()->sky.cmd_draw(cb, vv::ss()->swc.aspect());
      });

      vv::as()->post.render(cb, vv::ss()->swc, {
        .fog { 0.4, 0.6, 0.8, 2 },
        .far = 36,
      });
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::window_title = "poc-skybox";
}
