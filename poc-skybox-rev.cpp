#pragma leco app
#pragma leco add_resource_dir assets

import buffers;
import casein;
import dotz;
import hai;
import ofs;
import post;
import skybox;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct app_stuff {
  voo::device_and_queue dq { "poc-skybox-rev", casein::native_ptr, {
    .feats {
      .independentBlend = true,
      .samplerAnisotropy = true,
    },
  }};

  skybox::rev::pipeline sky {};
  post::pipeline post { dq, false };
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };

  ext_stuff() {
    vv::as()->post.update_descriptor_sets(vv::as()->sky.image_view());
    vv::as()->post.setup(swc);
  }
};

class scene_drawer : public buffers::vk::drawer {
  void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {}
  void edges(vee::command_buffer cb) override {}
public:
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();

    {
      voo::cmd_buf_one_time_submit ots { cb };

      scene_drawer scene {};

      vv::as()->sky.render(cb, &scene);
      vv::as()->post.render(cb, vv::ss()->swc, {});
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::window_title = "poc-skybox";
}
