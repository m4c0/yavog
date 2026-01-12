export module ofs;
import voo;
import wagen;

inline constexpr auto create_depth_attachment() {
  wagen::VkAttachmentDescription res{};
  res.format = VK_FORMAT_D32_SFLOAT;
  res.samples = VK_SAMPLE_COUNT_1_BIT;
  res.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  res.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  res.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  res.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  res.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  res.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  return res;
}

export namespace ofs {
  auto render_pass() {
    return vee::create_render_pass({
      .attachments {{
        vee::create_colour_attachment(
            VK_FORMAT_R8G8B8A8_SRGB,
            vee::image_layout_shader_read_only_optimal),
        vee::create_colour_attachment(
            VK_FORMAT_R32G32B32A32_SFLOAT,
            vee::image_layout_shader_read_only_optimal),
        vee::create_colour_attachment(
            VK_FORMAT_R32G32B32A32_SFLOAT,
            vee::image_layout_shader_read_only_optimal),
        create_depth_attachment(),
      }},
      .subpasses {{
        vee::create_subpass({
          .colours {{
            vee::create_attachment_ref(0, vee::image_layout_color_attachment_optimal),
            vee::create_attachment_ref(1, vee::image_layout_color_attachment_optimal),
            vee::create_attachment_ref(2, vee::image_layout_color_attachment_optimal),
          }},
          .depth_stencil = vee::create_attachment_ref(3, vee::image_layout_depth_stencil_attachment_optimal),
        }),
      }},
      .dependencies {{
        vee::create_colour_dependency(),
        vee::create_depth_dependency(),
      }},
    });
  }

  struct framebuffer {
    voo::bound_image colour;
    voo::bound_image position;
    voo::bound_image normal;
    voo::bound_image depth;
    vee::render_pass rp;
    vee::framebuffer fb;

    explicit framebuffer(vee::extent ext) :
      colour { voo::bound_image::create(
          ext, VK_FORMAT_R8G8B8A8_SRGB,
          vee::image_usage_colour_attachment, vee::image_usage_sampled) }
    , position { voo::bound_image::create(
          ext, VK_FORMAT_R32G32B32A32_SFLOAT,
          vee::image_usage_colour_attachment, vee::image_usage_sampled) }
    , normal { voo::bound_image::create(
          ext, VK_FORMAT_R32G32B32A32_SFLOAT,
          vee::image_usage_colour_attachment, vee::image_usage_sampled) }
    , depth { voo::bound_image::create_depth(ext, vee::image_usage_sampled) }
    , rp { render_pass() }
    , fb { vee::create_framebuffer({
      .render_pass = *rp,
      .attachments {{
        *colour.iv,
        *position.iv,
        *normal.iv,
        *depth.iv,
      }},
      .extent = ext,
    }) }
    {}
  };
}
