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
    .depthCompareOp   = VK_COMPARE_OP_GREATER_OR_EQUAL,
  });
}

class skybox {
  vee::render_pass m_rp = vee::create_render_pass({
    .attachments {{
      create_colour_attachment(),
      create_depth_attachment(),
    }},
    .subpasses {{
      vee::subpass({
        .colours {{
          vee::attachment_ref(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        }},
        .depth_stencil = vee::attachment_ref(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
      }),
    }},
    .dependencies {{
      vee::colour_dependency(),
      vee::depth_dependency(),
    }},
  });

  vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({
    vee::dsl_fragment_sampler(),
  });
  vee::descriptor_pool m_dpool = vee::create_descriptor_pool(1, {
    vee::combined_image_sampler(1)
  });
  vee::descriptor_set m_dset = vee::allocate_descriptor_set(*m_dpool, *m_dsl);

  vee::pipeline_layout m_pl = vee::create_pipeline_layout(*m_dsl);
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

  vee::sampler m_smp = vee::create_sampler(vee::linear_sampler);
  voo::bound_image m_img {};

  vee::framebuffer m_fb {};

public:
  skybox() {
    voo::load_image("3840px-Blue_Marble_2002.png", &m_img, [this](auto sz) {
      vee::update_descriptor_set(m_dset, 0, 0, *m_img.iv, *m_smp);
    });
  }
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
    vee::cmd_bind_gr_pipeline(cb, *m_ppl);
    vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset);
    vee::cmd_draw(cb, 3);
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

  post::pipeline post { dq, true };
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

      // This until it "ofs -> sky -> post" is the norm
      auto imb = vee::image_memory_barrier(*vv::as()->ofs.fb().colour.img);
      imb.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imb.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      imb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      imb.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      vee::cmd_pipeline_barrier(cb,
          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
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
