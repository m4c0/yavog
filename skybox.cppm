#pragma leco add_shader "skybox-fwd.vert"
#pragma leco add_shader "skybox-fwd.frag"
#pragma leco add_shader "skybox-rev.vert"
#pragma leco add_shader "skybox-rev.frag"
export module skybox;
import buffers;
import ofs;
import texmap;
import voo;

namespace skybox::fwd {
  inline constexpr auto create_colour_attachment() {
    return VkAttachmentDescription {
      .format         = VK_FORMAT_R8G8B8A8_UNORM,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
  }
  inline constexpr auto create_depth_attachment() {
    return VkAttachmentDescription {
      .format         = VK_FORMAT_D32_SFLOAT,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
  }

  inline constexpr auto depth() {
    return vee::depth::of({
      .depthTestEnable = true,
      .depthWriteEnable = false,
      .depthCompareOp = VK_COMPARE_OP_EQUAL,
    });
  }

  export class pipeline {
    struct upc {
      float aspect;
    };

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
    vee::descriptor_set m_dset = voo::allocate_descriptor_set("skybox-fwd", *m_dpool, *m_dsl);

    vee::pipeline_layout m_pl = vee::create_pipeline_layout(
        *m_dsl,
        vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline m_ppl = voo::create_graphics_pipeline("skybox-fwd", {
      .pipeline_layout = *m_pl,
      .render_pass = *m_rp,
      .back_face_cull = false,
      .depth { depth() },
      .blends { vee::colour_blend_none() },
    });

    vee::sampler m_smp = vee::create_sampler(vee::linear_sampler);

    vee::framebuffer m_fb {};

  public:
    struct setup_params {
      VkExtent2D ext;
      VkImageView colour;
      VkImageView depth;
      VkImageView output;
    };
    void setup(const setup_params & p) {
      m_fb = vee::create_framebuffer({
        .render_pass = *m_rp,
        .attachments {{
          p.colour,
          p.depth,
        }},
        .extent = p.ext,
      });

      vee::update_descriptor_set(m_dset, 0, 0, p.output, *m_smp);
    }

    void render(vee::command_buffer cb, const voo::swapchain & swc) {
      upc pc { .aspect = swc.aspect() };

      voo::cmd_render_pass rpg {vee::render_pass_begin{
        .command_buffer = cb,
        .render_pass = *m_rp,
        .framebuffer = *m_fb,
        .extent = swc.extent(),
      }, true};
      vee::cmd_set_viewport(cb, swc.extent());
      vee::cmd_set_scissor(cb, swc.extent());
      vee::cmd_bind_gr_pipeline(cb, *m_ppl);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset);
      vee::cmd_push_vertex_constants(cb, *m_pl, &pc);
      vee::cmd_draw(cb, 3);
    }
  };
}

namespace skybox::rev {
  inline auto create_render_pass() {
    unsigned view_mask = 0b111111;
    auto mv = vee::render_pass_multiview_create_info({
      .subpassCount = 1,
      .pViewMasks = &view_mask,
    });
    return vee::create_render_pass({
      .attachments {{
        vee::colour_attachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        vee::depth_attachment(),
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
      .next = &mv,
    });
  }
  inline auto image_view_create_info(VkImageAspectFlagBits aspect) {
    return VkImageViewCreateInfo {
      .viewType = VK_IMAGE_VIEW_TYPE_CUBE,
      .subresourceRange = vee::image_subresource_range(aspect, 6),
    };
  } 
  struct upc {
    float far;
    float near;
  };
  export class pipeline {
    static constexpr const unsigned size = 1024;
    static constexpr const VkExtent2D ext { size, size};
    voo::bound_image m_image = voo::bound_image::create(
        vee::cube_image_create_info(
          ext, VK_FORMAT_R8G8B8A8_UNORM,
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
        image_view_create_info(VK_IMAGE_ASPECT_COLOR_BIT));
    voo::bound_image m_depth = voo::bound_image::create(
        vee::cube_image_create_info(
          ext, VK_FORMAT_D32_SFLOAT,
          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT),
        image_view_create_info(VK_IMAGE_ASPECT_DEPTH_BIT));

    vee::render_pass m_rp = create_render_pass();

    vee::framebuffer m_fb = vee::create_framebuffer({
      .render_pass = *m_rp,
      .attachments {{
        *m_image.iv,
        *m_depth.iv,
      }},
      .extent = ext,
    });

    vee::descriptor_set_layout m_dsl = texmap::descriptor_set_layout();
    vee::pipeline_layout m_pl = vee::create_pipeline_layout(
        *m_dsl,
        vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline m_ppl = voo::create_graphics_pipeline("skybox-rev", {
      .pipeline_layout = *m_pl,
      .render_pass = *m_rp,
      .extent = ext,
      .back_face_cull = false,
      .depth = vee::depth::op_less(),
      .bindings = buffers::vk::ibindings(),
      .attributes = buffers::vk::iattrs(),
    });

  public:
    constexpr auto image_view() const { return *m_image.iv; }

    void render(vee::command_buffer cb, buffers::vk::drawer * drawer, upc pc) {
      voo::cmd_render_pass rp { vee::render_pass_begin { 
        .command_buffer = cb,
        .render_pass = *m_rp,
        .framebuffer = *m_fb,
        .extent = ext,
        .clear_colours {
          vee::clear_colour(0, 0, 0, 1),
          vee::clear_depth(1),
        },
      }, true };
      vee::cmd_bind_gr_pipeline(cb, *m_ppl);
      vee::cmd_push_vertex_constants(cb, *m_pl, &pc);
      drawer->faces(cb, *m_pl);
    }
  };
}

namespace skybox {
  export class pipeline {
    skybox::fwd::pipeline m_fwd {};
    skybox::rev::pipeline m_rev {};
    voo::single_cb m_cb {};

  public:
    void setup(voo::swapchain & swc, const ofs::pipeline & ofs) {
      m_fwd.setup({
        .ext = swc.extent(),
        .colour = *ofs.fb().colour.iv,
        .depth  = *ofs.fb().depth.iv,
        .output = m_rev.image_view(),
      });
    }

    void render_to_cubemap(buffers::vk::drawer * drawer, rev::upc pc) {
      auto cb = m_cb.cb();

      voo::run(voo::cmd_buf_one_time_submit { cb }, [&] {
        m_rev.render(cb, drawer, pc);
      });;
      voo::queue::universal()->queue_submit({
        .command_buffer = cb,
      });
    }

    void render(vee::command_buffer cb, voo::swapchain & swc) {
      m_fwd.render(cb, swc);
    }
  };
}
