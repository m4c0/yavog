export module ofs;
import silog;
import voo;
import wagen;

using namespace wagen;

inline constexpr auto create_depth_attachment(VkSampleCountFlagBits samples) {
  VkAttachmentDescription res{};
  res.format = VK_FORMAT_D32_SFLOAT;
  res.samples = VK_SAMPLE_COUNT_1_BIT;
  res.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  res.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  res.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  res.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  res.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  res.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  res.samples = samples;
  return res;
}

export namespace ofs {
  auto render_pass(VkSampleCountFlagBits samples) {
    return vee::create_render_pass({
      .attachments {{
        vee::create_colour_attachment({
          .format = VK_FORMAT_R8G8B8A8_UNORM,
          .final_layout = vee::image_layout_shader_read_only_optimal,
          .samples = samples,
        }),
        vee::create_colour_attachment({
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .final_layout = vee::image_layout_shader_read_only_optimal,
          .samples = samples,
        }),
        vee::create_colour_attachment({
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .final_layout = vee::image_layout_shader_read_only_optimal,
          .samples = samples,
        }),
        create_depth_attachment(samples),

        vee::create_colour_attachment({
          .format = VK_FORMAT_R8G8B8A8_UNORM,
          .final_layout = vee::image_layout_shader_read_only_optimal,
        }),
        vee::create_colour_attachment({
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .final_layout = vee::image_layout_shader_read_only_optimal,
        }),
        vee::create_colour_attachment({
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .final_layout = vee::image_layout_shader_read_only_optimal,
        }),
      }},
      .subpasses {{
        vee::create_subpass({
          .colours {{
            vee::create_attachment_ref(0, vee::image_layout_color_attachment_optimal),
            vee::create_attachment_ref(1, vee::image_layout_color_attachment_optimal),
            vee::create_attachment_ref(2, vee::image_layout_color_attachment_optimal),
          }},
          .depth_stencil = vee::create_attachment_ref(3, vee::image_layout_depth_stencil_attachment_optimal),
          .resolves {{
            vee::create_attachment_ref(4, vee::image_layout_color_attachment_optimal),
            vee::create_attachment_ref(5, vee::image_layout_color_attachment_optimal),
            vee::create_attachment_ref(6, vee::image_layout_color_attachment_optimal),
          }},
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

    auto img(vee::extent ext, VkFormat fmt, VkSampleCountFlagBits max_samples) {
      auto ci = vee::image_create_info(
          ext, fmt, 
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
      ci.samples = max_samples;
      return voo::bound_image::create(ci, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    auto dpth(vee::extent ext, VkSampleCountFlagBits max_samples) {
      auto ci = vee::depth_image_create_info(ext, VK_IMAGE_USAGE_SAMPLED_BIT);
      ci.samples = max_samples;
      return voo::bound_image::create(ci, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    explicit framebuffer(vee::extent ext, VkSampleCountFlagBits max_samples) :
      colour   { img(ext, VK_FORMAT_R8G8B8A8_UNORM,      max_samples) }
    , position { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT, max_samples) }
    , normal   { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT, max_samples) }
    , depth { dpth(ext, max_samples) }
    , rp { render_pass(max_samples) }
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
