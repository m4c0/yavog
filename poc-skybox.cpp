#pragma leco app
#pragma leco add_resource_dir assets

import buffers;
import casein;
import dotz;
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

class scene_drawer : public buffers::vk::drawer {
  texmap::cache txts {};
  buffers::all bufs { 16, models::cube::t {} };

  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
    vee::cmd_bind_descriptor_set(cb, pl, 0, txts.dset());
    bufs.cmd_draw_vtx(cb);
  }
  void edges(vee::command_buffer cb) override {}

public:
  scene_drawer() {
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
};

struct app_stuff {
  voo::device_and_queue dq { "poc-model", casein::native_ptr, {
    .feats {
      .independentBlend = true,
      .samplerAnisotropy = true,
    },
  }};
  scene_drawer scene {};

  post::pipeline post { dq, false };
  skybox::fwd::pipeline fwd {};
  skybox::rev::pipeline rev {};
  ofs::pipeline ofs {};
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };

  ext_stuff() {
    vv::as()->ofs.setup(swc);

    vv::as()->post.update_descriptor_sets(vv::as()->ofs);
    vv::as()->post.setup(swc);

    vv::as()->fwd.setup({
      .ext = swc.extent(),
      .colour = *vv::as()->ofs.fb().colour.iv,
      .depth = *vv::as()->ofs.fb().depth.iv,
      .output = vv::as()->rev.image_view(),
    });
  }
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();

    {
      voo::cmd_buf_one_time_submit ots { cb };
      vv::as()->rev.render(cb, &vv::as()->scene);

      vv::as()->ofs.render(cb, nullptr, {
        .light { dotz::normalise(dotz::vec3 { -1 }), 0 },
        .aspect = vv::ss()->swc.aspect(),
        .far = 32,
      });

      // This until it "ofs -> sky -> post" is the norm
      auto imb = vee::image_memory_barrier(*vv::as()->ofs.fb().colour.img);
      imb.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imb.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      imb.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      imb.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          imb);

      vv::as()->fwd.render(cb, vv::ss()->swc);

      vv::as()->post.render(cb, vv::ss()->swc, {
        .fog { 0.4, 0.6, 0.8, 0 },
        .far = 32,
      });
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::window_title = "poc-skybox";
}
