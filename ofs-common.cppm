export module ofs:common;
import dotz;
import jute;
import no;
import silog;
import sv;
import traits;
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
inline constexpr auto depth_attachment() {
  return VkAttachmentDescription {
    .format         = VK_FORMAT_D32_SFLOAT,
    .samples        = VK_SAMPLE_COUNT_1_BIT,
    .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };
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
  auto iv = vee::image_view_create_info({
    .image = *res.img,
    .format = fmt,
    .subresourceRange = vee::image_subresource_range(aspect),
  });
  res.iv = vee::image_view { &iv };

  return res;
}

inline auto create_render_pass(VkSampleCountFlagBits samples) {
  return vee::create_render_pass({
    .attachments {{
      msaa_attachment(VK_FORMAT_R8G8B8A8_UNORM,      samples),
      msaa_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, samples),
      msaa_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, samples),
      msaa_attachment(VK_FORMAT_D32_SFLOAT_S8_UINT,  samples),

      colour_attachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
      colour_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
      colour_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
      depth_attachment(),
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
          vee::attachment_ref(7, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
        }},
      }),
    }},
    .dependencies {{
      vee::colour_dependency(),
      vee::depth_dependency(),
    }},
  });
}

inline auto stencil(VkStencilOp stencil_op, VkCompareOp compare_op) {
  return VkStencilOpState {
    .failOp = VK_STENCIL_OP_KEEP,
    .passOp = stencil_op,
    .depthFailOp = VK_STENCIL_OP_KEEP,
    .compareOp = compare_op,
    .compareMask = ~0U,
    .writeMask = ~0U,
  };
}
inline auto create_graphics_pipeline(sv shader, vee::gr_pipeline_params p) {
  auto vert = voo::vert_shader(jute::fmt<"%s.vert.spv">(shader));
  auto frag = voo::frag_shader(jute::fmt<"%s.frag.spv">(shader));

  auto rp = create_render_pass(max_sampling());
  p.render_pass = *rp;
  p.multisampling = max_sampling();
  p.shaders = { *vert, *frag };
  return vee::create_graphics_pipeline(traits::move(p));
}
inline auto create_colour_only_pipeline(sv shader, vee::gr_pipeline_params p) {
  p.blends = {
    vee::colour_blend_classic(),
    VkPipelineColorBlendAttachmentState {},
    VkPipelineColorBlendAttachmentState {},
  };
  return create_graphics_pipeline(shader, p);
}

struct framebuffer : no::no {
  voo::bound_image msaa_colour;
  voo::bound_image msaa_position;
  voo::bound_image msaa_normal;

  voo::bound_image colour;
  voo::bound_image position;
  voo::bound_image normal;

  voo::bound_image msaa_depth;
  voo::bound_image depth;

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
  auto dpth(vee::extent ext) {
    auto ci = vee::image_create_info(
        ext, VK_FORMAT_D32_SFLOAT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    return voo::bound_image::create(ci, VK_IMAGE_ASPECT_DEPTH_BIT);
  }

  explicit framebuffer(vee::extent ext, VkSampleCountFlagBits max_samples = max_sampling()) :
    msaa_colour   { msaa_image(ext, VK_FORMAT_R8G8B8A8_UNORM,      max_samples) }
  , msaa_position { msaa_image(ext, VK_FORMAT_R32G32B32A32_SFLOAT, max_samples) }
  , msaa_normal   { msaa_image(ext, VK_FORMAT_R32G32B32A32_SFLOAT, max_samples) }

  , colour   { img(ext, VK_FORMAT_R8G8B8A8_UNORM,      VK_SAMPLE_COUNT_1_BIT) }
  , position { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT) }
  , normal   { img(ext, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT) }

  , msaa_depth { msaa_image(ext, VK_FORMAT_D32_SFLOAT_S8_UINT, max_samples) }
  , depth { dpth(ext) }

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
      *depth.iv,
    }},
    .extent = ext,
  }) }
  {
    silog::infof("Using MSAA %dx", max_samples);
  }
};

