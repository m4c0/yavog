export module ofs;
import clay;
import cube;
import hai;
import no;
import texmap;
import traits;
import silog;
import voo;
import wagen;

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
  res.format = VK_FORMAT_D32_SFLOAT;
  res.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  res.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  res.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  res.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  res.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  res.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  res.samples = samples;
  return res;
}
inline constexpr auto create_msaa_attachment(VkFormat fmt, VkSampleCountFlagBits samples) {
  auto final_layout = fmt == VK_FORMAT_D32_SFLOAT
    ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  return VkAttachmentDescription {
    .format         = fmt,
    .samples        = samples,
    .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout    = final_layout,
  };
}

static voo::bound_image create_msaa_image(vee::extent ext, VkFormat fmt, VkSampleCountFlagBits samples) {
  auto usage = fmt == VK_FORMAT_D32_SFLOAT
    ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  auto ci = vee::image_create_info(
      ext, fmt, 
      usage | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
  ci.samples = samples;

  voo::bound_image res {};
  res.img = vee::image { &ci };
  res.mem = vee::create_lazy_image_memory(wagen::physical_device(), *res.img);
  vee::bind_image_memory(*res.img, *res.mem);

  auto aspect = fmt == VK_FORMAT_D32_SFLOAT
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

namespace ofs {
  auto render_pass(VkSampleCountFlagBits samples) {
    return vee::create_render_pass({
      .attachments {{
        create_msaa_attachment(VK_FORMAT_R8G8B8A8_UNORM,      samples),
        create_msaa_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, samples),
        create_msaa_attachment(VK_FORMAT_R32G32B32A32_SFLOAT, samples),
        create_msaa_attachment(VK_FORMAT_D32_SFLOAT,          samples),

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
        create_depth_attachment(VK_SAMPLE_COUNT_1_BIT),
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
            vee::create_attachment_ref(7, vee::image_layout_color_attachment_optimal),
          }},
        }),
      }},
      .dependencies {{
        vee::create_colour_dependency(),
        vee::create_depth_dependency(),
      }},
    });
  }

  struct framebuffer : no::no {
    voo::bound_image msaa_colour;
    voo::bound_image msaa_position;
    voo::bound_image msaa_normal;

    voo::bound_image colour;
    voo::bound_image position;
    voo::bound_image normal;

    // TODO: remove depth resolve if we keep using pos.z
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
    auto dpth(vee::extent ext, VkSampleCountFlagBits max_samples) {
      auto flg = max_samples == VK_SAMPLE_COUNT_1_BIT
        ? VK_IMAGE_USAGE_SAMPLED_BIT
        : VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
      auto ci = vee::depth_image_create_info(ext, flg);
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

    , msaa_depth { create_msaa_image(ext, VK_FORMAT_D32_SFLOAT, max_samples) }
    , depth { dpth(ext, VK_SAMPLE_COUNT_1_BIT) }

    , rp { render_pass(max_samples) }
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

  struct upc {
    float aspect;
    float fov = 90;
  };
  export class pipeline : no::no {
    vee::pipeline_layout m_pl = vee::create_pipeline_layout(
        *texmap::descriptor_set_layout(),
        vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline m_ppl = vee::create_graphics_pipeline({
      .pipeline_layout = *m_pl,
      .render_pass = *render_pass(max_sampling()),
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .multisampling = max_sampling(),
      .depth = vee::depth::op_less(),
      .blends {
        vee::colour_blend_classic(),
        vee::colour_blend_none(),
        vee::colour_blend_none(),
      },
      .shaders {
        *clay::vert_shader("poc-mcish", [] {}),
        *clay::frag_shader("poc-mcish", [] {}),
      },
      .bindings {
        cube::v_buffer::vertex_input_bind(),
        cube::i_buffer::vertex_input_bind_per_instance(),
      },
      .attributes { 
        vee::vertex_attribute_vec3(0, traits::offset_of(&cube::vtx::pos)),
        vee::vertex_attribute_vec2(0, traits::offset_of(&cube::vtx::uv)),
        vee::vertex_attribute_vec3(0, traits::offset_of(&cube::vtx::normal)),
        vee::vertex_attribute_vec4(1, traits::offset_of(&cube::inst::pos)),
      },
    });

    upc m_pc {};
    vee::extent m_ext {};
    hai::uptr<framebuffer> m_fb {};

  public:
    [[nodiscard]] constexpr const auto & fb() const { return *m_fb; }

    void setup(const voo::swapchain & swc) {
      m_pc.aspect = swc.aspect();
      m_ext = swc.extent();
      m_fb.reset(new framebuffer { swc.extent() });
    }

    [[nodiscard]] auto cmd_render_pass(vee::command_buffer cb, vee::descriptor_set tmap) {
      voo::cmd_render_pass rp { vee::render_pass_begin {
        .command_buffer = cb,
        .render_pass = *m_fb->rp,
        .framebuffer = *m_fb->fb,
        .extent = m_ext,
        .clear_colours { 
          vee::clear_colour({ 0, 0, 0, 1 }), 
          vee::clear_colour({ 0, 0, 10, 0 }), 
          vee::clear_colour({ 0, 0, 0, 0 }), 
          vee::clear_depth(1.0),
          vee::clear_colour({ 0, 0, 0, 1 }), 
          vee::clear_colour({ 0, 0, 10, 0 }), 
          vee::clear_colour({ 0, 0, 0, 0 }), 
          vee::clear_depth(1.0),
        },
      }, true };
      vee::cmd_set_viewport(cb, m_ext);
      vee::cmd_set_scissor(cb, m_ext);
      vee::cmd_bind_gr_pipeline(cb, *m_ppl);
      vee::cmd_push_vertex_constants(cb, *m_pl, &m_pc);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, tmap);
      return rp;
    }
  };
}
