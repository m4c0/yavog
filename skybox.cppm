#pragma leco add_shader "skybox.vert"
#pragma leco add_shader "skybox.frag"
export module skybox;
import ofs;
import voo;

namespace skybox::fwd {
  inline constexpr auto create_colour_attachment() {
    return VkAttachmentDescription {
      .format         = VK_FORMAT_R8G8B8A8_UNORM,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
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
      .depthTestEnable = vk_true,
      .depthCompareOp  = VK_COMPARE_OP_GREATER_OR_EQUAL,
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
    vee::descriptor_set m_dset = vee::allocate_descriptor_set(*m_dpool, *m_dsl);

    vee::pipeline_layout m_pl = vee::create_pipeline_layout(
        *m_dsl,
        vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline m_ppl = vee::create_graphics_pipeline({
      .pipeline_layout = *m_pl,
      .render_pass = *m_rp,
      .back_face_cull = false,
      .depth = depth(),
      .blends { vee::colour_blend_none() },
      .shaders {
        *voo::vert_shader("skybox.vert.spv"),
        *voo::frag_shader("skybox.frag.spv"),
      },
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
    void setup(const voo::swapchain & swc, const setup_params & p) {
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
      vee::cmd_bind_gr_pipeline(cb, *m_ppl);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset);
      vee::cmd_push_vertex_constants(cb, *m_pl, &pc);
      vee::cmd_draw(cb, 3);
    }
  };

}
