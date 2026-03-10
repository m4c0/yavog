#pragma leco app
#pragma leco add_shader "poc-skybox.vert"
#pragma leco add_shader "poc-skybox.frag"
#pragma leco add_resource_dir assets

import casein;
import dotz;
import ofs;
import post;
import vinyl;
import voo;

inline constexpr auto create_colour_attachment() {
  return VkAttachmentDescription {
    .format = VK_FORMAT_R8G8B8A8_UNORM,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
}
inline constexpr auto create_depth_attachment() {
  return VkAttachmentDescription {
    .format = VK_FORMAT_D32_SFLOAT,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };
}
inline constexpr auto depth() {
  return vee::depth::of({
    .depthTestEnable  = vk_true,
    .depthCompareOp   = VK_COMPARE_OP_GREATER,
  });
}

class skybox {
  vee::render_pass m_rp = vee::create_render_pass({
    .attachments {{
      create_colour_attachment(),
      create_depth_attachment(),
    }},
    .subpasses {{
      vee::create_subpass({
        .colours {{
          vee::create_attachment_ref(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        }},
        .depth_stencil = vee::create_attachment_ref(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
      }),
    }},
    .dependencies {{
      vee::create_colour_dependency(),
      vee::create_depth_dependency(),
    }},
  });
  vee::pipeline_layout m_pl = vee::create_pipeline_layout();
  vee::gr_pipeline m_ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *m_pl,
    .render_pass = *m_rp,
    .back_face_cull = false,
    .depth = depth(),
    .blends { vee::colour_blend_none() },
    .shaders {
      *voo::vert_shader("poc-skybox.vert.spv"),
      *voo::frag_shader("poc-skybox.frag.spv"),
    },
  });

  vee::framebuffer m_fb {};

public:
  void setup(const voo::swapchain & swc, const ofs::pipeline & ofs) {
    m_fb = vee::create_framebuffer({
      .render_pass = *m_rp,
      .attachments {{
        *ofs.fb().colour.iv,
        *ofs.fb().depth.iv,
      }},
      .extent = swc.extent(),
    });
  }

  void render(vee::command_buffer cb, const voo::swapchain & swc) {
    voo::cmd_render_pass rpg {vee::render_pass_begin{
      .command_buffer = cb,
      .render_pass = *m_rp,
      .framebuffer = *m_fb,
      .extent = swc.extent(),
    }, true};
  }
};

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

class scene_drawer : public ofs::drawer {
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

  post::pipeline post { dq };
  skybox sky {};
  ofs::pipeline ofs {};
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };

  ext_stuff() {
    vv::as()->ofs.setup(swc);

    vv::as()->sky.setup(swc, vv::as()->ofs);

    vv::as()->post.update_descriptor_sets(vv::as()->ofs);
    vv::as()->post.setup(swc);
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

      vv::as()->sky.render(cb, vv::ss()->swc);

      vv::as()->post.render(cb, vv::ss()->swc, {
        .fog { 0.4, 0.6, 0.8, 2 },
        .far = 32,
      });
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  casein::window_title = "poc-skybox";
}
