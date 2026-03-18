export module msaa;
import no;
import silog;
import traits;
import voo;

namespace msaa {
  inline VkSampleCountFlagBits max_sampling() {
    auto lim = vee::get_physical_device_properties().limits;
    auto max = lim.framebufferColorSampleCounts & lim.framebufferDepthSampleCounts;
    // if (max & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
    // if (max & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
    // if (max & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
    // if (max & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
    if (max & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
    if (max & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
    return VK_SAMPLE_COUNT_1_BIT;
  }

  inline voo::bound_image msaa_image(vee::extent ext, VkFormat fmt, VkSampleCountFlagBits samples) {
    auto usage = fmt == VK_FORMAT_D32_SFLOAT_S8_UINT
      ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
      : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto ci = vee::image_create_info(
        ext, fmt, 
        usage | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    ci.samples = samples;

    voo::bound_image res {};
    res.img = vee::image { &ci };
    res.mem = vee::create_lazy_image_memory(wagen::physical_device(), *res.img);
    vee::bind_image_memory(*res.img, *res.mem);

    auto aspect = fmt == VK_FORMAT_D32_SFLOAT_S8_UINT
      ? VK_IMAGE_ASPECT_DEPTH_BIT
      : VK_IMAGE_ASPECT_COLOR_BIT;
    res.iv = vee::create_image_view({
      .image = *res.img,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = fmt,
      .subresourceRange = vee::image_subresource_range(aspect),
    });

    return res;
  }
  inline constexpr auto msaa_attachment(VkFormat fmt, VkSampleCountFlagBits samples) {
    auto final_layout = fmt == VK_FORMAT_D32_SFLOAT_S8_UINT
      ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
      : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    return VkAttachmentDescription {
      .format         = fmt,
      .samples        = samples,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout    = final_layout,
    };
  }

  inline constexpr auto colour_attachment(VkFormat fmt, VkImageLayout fl) {
    return VkAttachmentDescription {
      .format         = fmt,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout    = fl,
    };
  }
  inline auto create_render_pass(VkSampleCountFlagBits samples = max_sampling()) {
    return vee::create_render_pass({
      .attachments {{
        msaa_attachment(VK_FORMAT_R8G8B8A8_UNORM,      samples),
        msaa_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, samples),
        msaa_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, samples),
        msaa_attachment(VK_FORMAT_D32_SFLOAT_S8_UINT,  samples),

        colour_attachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        colour_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        colour_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
      }},
      .subpasses {{
        vee::subpass({
          .colours {{
            vee::attachment_ref(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
            vee::attachment_ref(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
            vee::attachment_ref(2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
          }},
          .depth_stencil = vee::attachment_ref(3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
          .resolves {{
            vee::attachment_ref(4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
            vee::attachment_ref(5, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
            vee::attachment_ref(6, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
          }},
        }),
      }},
      .dependencies {{
        vee::colour_dependency(),
        vee::depth_dependency(),
      }},
    });
  }

  export auto create_graphics_pipeline(vee::gr_pipeline_params p) {
    auto rp = create_render_pass();
    p.render_pass = *rp;
    p.multisampling = max_sampling();
    return vee::create_graphics_pipeline(traits::move(p));
  }

  export class framebuffer : no::no {
    voo::bound_image m_msaa_colour;
    voo::bound_image m_msaa_position;
    voo::bound_image m_msaa_normal;

    voo::bound_image m_colour;
    voo::bound_image m_position;
    voo::bound_image m_normal;

    voo::bound_image m_msaa_depth;
    voo::bound_image m_depth;

    vee::render_pass m_rp;
    vee::framebuffer m_fb;
    vee::extent m_ext;

    auto img(vee::extent ext, VkFormat fmt, VkSampleCountFlagBits max_samples) {
      auto flg = max_samples == VK_SAMPLE_COUNT_1_BIT
        ? VK_IMAGE_USAGE_SAMPLED_BIT
        : VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
      auto ci = vee::image_create_info(
          ext, fmt, 
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | flg);
      ci.samples = max_samples;
      return voo::bound_image::create(ci, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    auto dpth(vee::extent ext) {
      auto ci = vee::image_create_info(
          ext, VK_FORMAT_D32_SFLOAT,
          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
      return voo::bound_image::create(ci, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

  public:
    explicit framebuffer(vee::extent ext, VkSampleCountFlagBits max_samples = max_sampling()) :
      m_msaa_colour   { msaa_image(ext, VK_FORMAT_R8G8B8A8_UNORM,      max_samples) }
    , m_msaa_position { msaa_image(ext, VK_FORMAT_R32G32B32A32_SFLOAT, max_samples) }
    , m_msaa_normal   { msaa_image(ext, VK_FORMAT_R32G32B32A32_SFLOAT, max_samples) }

    , m_colour   { img(ext, VK_FORMAT_R8G8B8A8_UNORM,      VK_SAMPLE_COUNT_1_BIT) }
    , m_position { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT) }
    , m_normal   { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT) }

    , m_msaa_depth { msaa_image(ext, VK_FORMAT_D32_SFLOAT_S8_UINT, max_samples) }
    , m_depth { dpth(ext) }

    , m_rp { create_render_pass(max_samples) }
    , m_fb { vee::create_framebuffer({
      .render_pass = *m_rp,
      .attachments {{
        *m_msaa_colour.iv,
        *m_msaa_position.iv,
        *m_msaa_normal.iv,
        *m_msaa_depth.iv,
        *m_colour.iv,
        *m_position.iv,
        *m_normal.iv,
      }},
      .extent = ext,
    }) }
    , m_ext { ext }
    {
      silog::infof("Using MSAA %dx", max_samples);
    }

    [[nodiscard]] auto colour()   const { return *m_colour.iv;   }
    [[nodiscard]] auto normal()   const { return *m_normal.iv;   }
    [[nodiscard]] auto position() const { return *m_position.iv; }

    void cmd_render_pass(vee::command_buffer cb, float far, auto && fn) {
      voo::cmd_render_pass rp { vee::render_pass_begin {
        .command_buffer = cb,
        .render_pass = *m_rp,
        .framebuffer = *m_fb,
        .extent = m_ext,
        .clear_colours { 
          vee::clear_colour({ 0, 0, 0, 1 }), 
          vee::clear_colour({ 0, 0, far, 0 }), 
          vee::clear_colour({ 0, 0, 0, 0 }), 
          vee::clear_depth(1.0),
          vee::clear_colour({ 0, 0, 0, 1 }), 
          vee::clear_colour({ 0, 0, far, 0 }), 
          vee::clear_colour({ 0, 0, 0, 0 }), 
        },
      }, true };
      vee::cmd_set_viewport(cb, m_ext);
      vee::cmd_set_scissor(cb, m_ext);
      fn();
    }
  };
}
