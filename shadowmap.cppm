#pragma leco add_shader "shadowmap.frag"
#pragma leco add_shader "shadowmap.vert"
export module shadowmap;
import clay;
import cube;
import no;
import traits;
import voo;
import wagen;

using namespace wagen;

inline constexpr auto create_depth_attachment(VkSampleCountFlagBits samples) {
  return VkAttachmentDescription {
    .format         = VK_FORMAT_D32_SFLOAT,
    .samples        = VK_SAMPLE_COUNT_1_BIT,
    .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
}

namespace shadowmap {
  static constexpr const vee::extent ext { 1024, 1024 };

  export class pipeline : no::no {
    voo::bound_image m_depth = voo::bound_image::create_depth(
        ext,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    vee::render_pass m_rp = vee::create_render_pass({
      .attachments {{
        create_depth_attachment(VK_SAMPLE_COUNT_1_BIT),
      }},
      .subpasses {{
        vee::create_subpass({
          .depth_stencil = vee::create_attachment_ref(0, vee::image_layout_depth_stencil_attachment_optimal),
        }),
      }},
      .dependencies {{
        vee::create_depth_dependency(),
      }},
    });
    vee::framebuffer m_fb = vee::create_framebuffer({
      .render_pass = *m_rp,
      .attachments {{ *m_depth.iv }},
      .extent = ext,
    });

    vee::pipeline_layout m_pl = vee::create_pipeline_layout();
    vee::gr_pipeline m_ppl = vee::create_graphics_pipeline({
      .pipeline_layout = *m_pl,
      .render_pass = *m_rp,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .depth = vee::depth::op_less(),
      .blends {},
      .shaders {
        *clay::vert_shader("shadowmap", [] {}),
        *clay::frag_shader("shadowmap", [] {}),
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

  public:
    [[nodiscard]] auto cmd_render_pass(vee::command_buffer cb) {
      voo::cmd_render_pass rp { vee::render_pass_begin {
        .command_buffer = cb,
        .render_pass = *m_rp,
        .framebuffer = *m_fb,
        .extent = ext,
        .clear_colours { vee::clear_depth(1.0) },
      }, true };
      vee::cmd_set_viewport(cb, ext);
      vee::cmd_set_scissor(cb,  ext);
      vee::cmd_bind_gr_pipeline(cb, *m_ppl);
      return rp;
    }

    [[nodiscard]] constexpr const auto iv() const { return *m_depth.iv; }
  };
}
