#pragma leco app
#pragma leco add_resource_dir assets

import buffers;
import casein;
import dotz;
import hai;
import models;
import ofs;
import post;
import skybox;
import texmap;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

static inline auto device_and_queue() {
  auto dn = vee::physical_device_vulkan11_features({
    .multiview = true,
  });
  return voo::device_and_queue { "poc-skybox-rev", casein::native_ptr, {
    .feats {
      .independentBlend = true,
      .samplerAnisotropy = true,
    },
    .next = &dn,
  }};
}

struct app_stuff : buffers::vk::drawer {
  voo::device_and_queue dq = device_and_queue();

  texmap::cache txts {};
  buffers::all bufs { 16, models::cube::t {} };

  skybox::rev::pipeline sky {};
  post::pipeline post { dq, false };

  app_stuff() {
    float txt0 = txts.load("Tiles040_1K-JPG_Color.jpg");
    float txt1 = txts.load("Tiles101_1K-JPG_Color.jpg");
    float txt2 = txts.load("Tiles131_1K-JPG_Color.jpg");
    float txt3 = txts.load("Ground037_1K-JPG_Color.jpg");

    auto i = bufs.inst.map();
    auto cube = [&](dotz::vec3 p, float txt) {
      i += {
        .pos = p,
        .mdl = 1,
        .size { 1 },
        .txtid = txt,
      };
    };
    cube({ 0, 0, 4 }, txt0);
    cube({ 0, 4, 0 }, txt1);
    cube({ 4, 0, 0 }, txt2);
    cube({ 0, 0, -4 }, txt3);
    cube({ 0, -4, 0 }, txt3);
    cube({ -4, 0, 0 }, txt3);

    auto v = bufs.vcmd.map(nullptr);
    v[1].instanceCount = bufs.inst.count();
  }

  virtual ~app_stuff() = default;
  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    vee::cmd_bind_descriptor_set(cb, pl, 0, txts.dset());
    bufs.cmd_draw_vtx(cb);
  }
  void edges(vee::command_buffer cb) override {}
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };

  ext_stuff() {
    vv::as()->post.update_descriptor_sets(vv::as()->sky.image_view());
    vv::as()->post.setup(swc);
  }
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();

    {
      voo::cmd_buf_one_time_submit ots { cb };
      vv::as()->sky.render(cb, vv::as());
      vv::as()->post.render(cb, vv::ss()->swc, {});
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::window_title = "poc-skybox";
}
