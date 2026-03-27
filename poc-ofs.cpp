#pragma leco app
#pragma leco add_resource_dir assets
import buffers;
import chunk;
import casein;
import dotz;
import models;
import msaa;
import ofs;
import post;
import texmap;
import vinyl;
import voo;

using namespace wagen;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct app_stuff {
  voo::device_and_queue dq { "poc-ofs", casein::native_ptr, {
    .feats {
      .independentBlend = true,
      .samplerAnisotropy = true,
    },
  }};
  vee::render_pass rp = voo::single_att_render_pass(dq);

  post::pipeline post { dq };
  ofs::pipeline ofs {};

  voo::single_cb scb {};

  texmap::cache tmap {};

  chunk::buffers bufs {
    5,
    models::corner::t {},
    models::cube::t {},
    models::prism::t {},
  };

  app_stuff() {
    float txt_id = tmap.load("Tiles101_1K-JPG_Color.pci");

    auto m = bufs.inst.map();
    m += { .pos { -1,  0, 4 }, .txtid = txt_id };
    m += { .pos {  1,  0, 4 }, .txtid = txt_id };
    m += { .pos { -1,  0, 2 }, .txtid = txt_id };
    m += { .pos {  1,  0, 2 }, .txtid = txt_id };
    m += { .pos {  0, -1, 3 }, .txtid = txt_id };
    
    auto vc = bufs.vcmd.map(nullptr);
    vc[2].instanceCount = 5;
    
    auto ec = bufs.ecmd.map(nullptr);
    ec[2].instanceCount = 5;
  }
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };
  msaa::framebuffer msaa { swc.extent() };

  ext_stuff() {
    vv::as()->post.update_descriptor_sets(msaa);
    vv::as()->post.setup(swc);
  }
};

struct scene_drawer : public buffers::vk::drawer {
  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    if (pl) vee::cmd_bind_descriptor_set(cb, pl, 0, vv::as()->tmap.dset());
    vv::as()->bufs.cmd_draw_vtx(cb);
  }
  void edges(vee::command_buffer cb) override {
    vv::as()->bufs.cmd_draw_edg(cb);
  }
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();

    auto l = dotz::normalise(dotz::vec3 { 1, -0.3, 0.3 });
    scene_drawer d {};

    auto cb = vv::ss()->cb.cb();
    {
      voo::cmd_buf_one_time_submit ots { cb };
      vv::ss()->msaa.cmd_render_pass(cb, 100, [&] {
        vv::as()->ofs.render(cb, &d, {
          .light { l, 0 },
          .aspect = vv::ss()->swc.aspect(),
          .far = 100.0f,
        });
      });
      vv::as()->post.render(cb, vv::ss()->swc, {
        .fog { 0.4, 0.6, 0.8, 2 },
        .far = 100.f,
      });
    }

    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::handle(casein::KEY_DOWN, casein::K_F, [] {
    casein::fullscreen = !casein::fullscreen;
    casein::interrupt(casein::IRQ_FULLSCREEN);
  });

  casein::window_title = "poc-chunk";
}
