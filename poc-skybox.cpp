#pragma leco app
#pragma leco add_resource_dir assets

import buffers;
import casein;
import dotz;
import ofs;
import post;
import skybox;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

class scene_drawer : public buffers::vk::drawer {
  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {}
  void edges(vee::command_buffer cb) override {}
};

struct app_stuff {
  voo::device_and_queue dq { "poc-model", casein::native_ptr, {
    .feats {
      .independentBlend = true,
      .samplerAnisotropy = true,
    },
  }};
  scene_drawer scene {};

  post::pipeline post { dq, true };
  skybox::fwd::pipeline sky {};
  ofs::pipeline ofs {};
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };
  voo::bound_image earth {};

  ext_stuff() {
    vv::as()->ofs.setup(swc);

    vv::as()->post.update_descriptor_sets(vv::as()->ofs);
    vv::as()->post.setup(swc);

    voo::load_image("3840px-Blue_Marble_2002.png", &earth, [this](auto sz) {
      vv::as()->sky.setup(swc, {
        .ext = swc.extent(),
        .colour = *vv::as()->ofs.fb().colour.iv,
        .depth = *vv::as()->ofs.fb().depth.iv,
        .output = *earth.iv,
      });
    });
  }
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();

    {
      voo::cmd_buf_one_time_submit ots { cb };

      vv::as()->ofs.render(cb, &vv::as()->scene, {
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

      vv::as()->sky.render(cb, vv::ss()->swc);

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
