export module ofs;
import silog;
import voo;
import wagen;

using namespace wagen;

inline constexpr auto create_depth_attachment() {
  VkAttachmentDescription res{};
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

export namespace ofs {
  auto render_pass() {
    return vee::create_render_pass({
      .attachments {{
        vee::create_colour_attachment(
            VK_FORMAT_R8G8B8A8_UNORM,
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
    VkSampleCountFlagBits max_samples = max_sampling();
    voo::bound_image colour;
    voo::bound_image position;
    voo::bound_image normal;
    voo::bound_image depth;
    vee::render_pass rp;
    vee::framebuffer fb;

    static auto img(vee::extent ext, VkFormat fmt) {
      auto ci = vee::image_create_info(
          ext, fmt, 
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
      return voo::bound_image::create(ci, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    explicit framebuffer(vee::extent ext) :
      colour   { img(ext, VK_FORMAT_R8G8B8A8_UNORM)      }
    , position { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT) }
    , normal   { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT) }
    , depth { voo::bound_image::create_depth(ext, VK_IMAGE_USAGE_SAMPLED_BIT) }
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
    {
      silog::infof("Using MSAA %dx", max_samples);
    }
  };
}
