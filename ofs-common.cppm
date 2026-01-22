export module ofs:common;
import no;
import silog;
import voo;

using namespace wagen;

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

inline constexpr auto create_depth_attachment(VkSampleCountFlagBits samples) {
  VkAttachmentDescription res{};
  res.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
  res.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  res.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  res.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  res.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  res.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  res.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  res.samples = samples;
  return res;
}
inline constexpr auto create_msaa_attachment(VkFormat fmt, VkSampleCountFlagBits samples) {
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

inline voo::bound_image create_msaa_image(vee::extent ext, VkFormat fmt, VkSampleCountFlagBits samples) {
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
  auto iv = vee::image_view_create_info({
    .image = *res.img,
    .format = fmt,
    .subresourceRange = vee::image_subresource_range(aspect),
  });
  res.iv = vee::image_view { &iv };

  return res;
}

inline vee::sampler create_shadowmap_sampler() {
  return vee::create_sampler({
    .magFilter = VK_FILTER_LINEAR,
    .minFilter = VK_FILTER_LINEAR,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
  });
}

inline auto create_render_pass(VkSampleCountFlagBits samples) {
  return vee::create_render_pass({
    .attachments {{
      create_msaa_attachment(VK_FORMAT_R8G8B8A8_UNORM,      samples),
      create_msaa_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, samples),
      create_msaa_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, samples),
      create_msaa_attachment(VK_FORMAT_D32_SFLOAT_S8_UINT,  samples),

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
      vee::create_subpass({
        .depth_stencil = vee::create_attachment_ref(3, vee::image_layout_depth_stencil_attachment_optimal),
        .preserves {{ 4, 5, 6 }},
      }),
      vee::create_subpass({
        .colours {{
          vee::create_attachment_ref(0, vee::image_layout_color_attachment_optimal),
        }},
        .inputs {{
          vee::create_attachment_ref(0, vee::image_layout_shader_read_only_optimal),
        }},
        .depth_stencil = vee::create_attachment_ref(3, vee::image_layout_depth_stencil_attachment_optimal),
        .resolves {{
          vee::create_attachment_ref(4, vee::image_layout_color_attachment_optimal),
        }},
        .preserves {{ 5, 6 }},
      }),
    }},
    .dependencies {{
      vee::create_dependency({
        .src_subpass = 0,
        .src_stage_mask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .src_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dst_subpass = 1,
        .dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dst_access_mask =
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        .dependency = 0,
      }),
      vee::create_dependency({
        .src_subpass = 1,
        .src_stage_mask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .src_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dst_subpass = 2,
        .dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dst_access_mask =
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependency = 0,
      }),
      vee::create_dependency({
        .src_subpass = 2,
        .src_stage_mask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .src_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dst_subpass = vk_subpass_external,
        .dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dst_access_mask =
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependency = 0,
      }),
      vee::create_dependency({
        .src_subpass = 0,
        .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .src_access_mask =
          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dst_subpass = 2,
        .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .dst_access_mask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
        .dependency = VK_DEPENDENCY_BY_REGION_BIT,
      }),
    }},
  });
}

struct upc {
  float aspect;
  float fov = 90;
  float sun_angle;
};

struct framebuffer : no::no {
  voo::bound_image msaa_colour;
  voo::bound_image msaa_position;
  voo::bound_image msaa_normal;

  voo::bound_image colour;
  voo::bound_image position;
  voo::bound_image normal;

  voo::bound_image msaa_depth;

  vee::render_pass rp;
  vee::framebuffer fb;

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
  auto dpth(vee::extent ext, VkSampleCountFlagBits max_samples) {
    auto flg = max_samples == VK_SAMPLE_COUNT_1_BIT
      ? VK_IMAGE_USAGE_SAMPLED_BIT
      : VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    auto ci = vee::image_create_info(
        ext, VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | flg);
    ci.samples = max_samples;
    return voo::bound_image::create(ci, VK_IMAGE_ASPECT_DEPTH_BIT);
  }

  explicit framebuffer(vee::extent ext, VkSampleCountFlagBits max_samples = max_sampling()) :
    msaa_colour   { create_msaa_image(ext, VK_FORMAT_R8G8B8A8_UNORM,      max_samples) }
  , msaa_position { create_msaa_image(ext, VK_FORMAT_R32G32B32A32_SFLOAT, max_samples) }
  , msaa_normal   { create_msaa_image(ext, VK_FORMAT_R32G32B32A32_SFLOAT, max_samples) }

  , colour   { img(ext, VK_FORMAT_R8G8B8A8_UNORM,      VK_SAMPLE_COUNT_1_BIT) }
  , position { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT) }
  , normal   { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT) }

  , msaa_depth { create_msaa_image(ext, VK_FORMAT_D32_SFLOAT_S8_UINT, max_samples) }

  , rp { create_render_pass(max_samples) }
  , fb { vee::create_framebuffer({
    .render_pass = *rp,
    .attachments {{
      *msaa_colour.iv,
      *msaa_position.iv,
      *msaa_normal.iv,
      *msaa_depth.iv,
      *colour.iv,
      *position.iv,
      *normal.iv,
    }},
    .extent = ext,
  }) }
  {
    silog::infof("Using MSAA %dx", max_samples);
  }
};

