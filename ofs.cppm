#pragma leco add_shader "ofs-colour.frag"
#pragma leco add_shader "ofs-colour.vert"
#pragma leco add_shader "ofs-lights.frag"
#pragma leco add_shader "ofs-lights.vert"
#pragma leco add_shader "ofs-planes.frag"
#pragma leco add_shader "ofs-planes.vert"
#pragma leco add_shader "ofs-shadow.frag"
#pragma leco add_shader "ofs-shadow.vert"
export module ofs;
export import :common;
import clay;
import cube;
import hai;
import texmap;
import traits;
import voo;

using namespace wagen;

namespace ofs {
  struct planes : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline ppl = create_graphics_pipeline("ofs-planes", {
      .pipeline_layout = *pl,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
      .back_face_cull = false,
      .depth = vee::depth::op_less(),
      .blends {
        vee::colour_blend_classic(),
        VkPipelineColorBlendAttachmentState {},
        VkPipelineColorBlendAttachmentState {},
      },
    });
  };
  struct colour : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(
      *texmap::descriptor_set_layout(),
      vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline ppl = create_graphics_pipeline("ofs-colour", {
      .pipeline_layout = *pl,
      .depth = vee::depth::op_less(),
      .blends {
        vee::colour_blend_classic(),
        vee::colour_blend_none(),
        vee::colour_blend_none(),
      },
      .bindings {
        cube::v_buffer::vertex_input_bind(),
        cube::i_buffer::vertex_input_bind_per_instance(),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, traits::offset_of(&cube::vtx::pos)),
        vee::vertex_attribute_vec2(0, traits::offset_of(&cube::vtx::uv)),
        vee::vertex_attribute_vec3(0, traits::offset_of(&cube::vtx::normal)),
        vee::vertex_attribute_vec4(1, traits::offset_of(&cube::inst::pos)),
      },
    });
  };
  struct shadow : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline ppl = create_graphics_pipeline("ofs-shadow", {
      .pipeline_layout = *pl,
      .back_face_cull = false,
      .depth = vee::depth::of({
        .depthTestEnable = vk_true,
        .depthWriteEnable = vk_false,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencilTestEnable = vk_true,
        .front = stencil(VK_STENCIL_OP_INCREMENT_AND_WRAP, VK_COMPARE_OP_ALWAYS),
        .back  = stencil(VK_STENCIL_OP_DECREMENT_AND_WRAP, VK_COMPARE_OP_ALWAYS),
      }),
      .blends {
        vee::colour_blend_classic(),
        VkPipelineColorBlendAttachmentState {},
        VkPipelineColorBlendAttachmentState {},
      },
      .bindings {
        vee::vertex_input_bind(sizeof(dotz::vec4)),
        cube::i_buffer::vertex_input_bind_per_instance(),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, 0),
        vee::vertex_attribute_vec4(1, traits::offset_of(&cube::inst::pos)),
      },
    });
  };
  struct lights : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(
      *texmap::descriptor_set_layout(),
      vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline ppl = create_graphics_pipeline("ofs-lights", {
      .pipeline_layout = *pl,
      .depth = vee::depth::of({
        .depthTestEnable = vk_true,
        .depthWriteEnable = vk_false,
        .depthCompareOp = VK_COMPARE_OP_EQUAL,
        .stencilTestEnable = vk_true,
        .front = stencil(VK_STENCIL_OP_KEEP, VK_COMPARE_OP_EQUAL),
        .back  = stencil(VK_STENCIL_OP_KEEP, VK_COMPARE_OP_EQUAL),
      }),
      .blends {
        vee::colour_blend_classic(),
        VkPipelineColorBlendAttachmentState {},
        VkPipelineColorBlendAttachmentState {},
      },
      .bindings {
        cube::v_buffer::vertex_input_bind(),
        cube::i_buffer::vertex_input_bind_per_instance(),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, traits::offset_of(&cube::vtx::pos)),
        vee::vertex_attribute_vec2(0, traits::offset_of(&cube::vtx::uv)),
        vee::vertex_attribute_vec4(1, traits::offset_of(&cube::inst::pos)),
      },
    });
  };
}

namespace ofs {
  export struct params {
    vee::buffer::type vtx;
    vee::buffer::type inst;
    vee::buffer::type idx;
    vee::buffer::type shdvtx;
    vee::buffer::type shdidx;
    unsigned sicount;
    unsigned icount;

    vee::descriptor_set tmap;
    dotz::vec4 light;
  };

  export class pipeline {
    planes m_pln {};
    colour m_clr {};
    shadow m_shd {};
    lights m_lig {};

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

    void render(vee::command_buffer cb, const params & p) {
      m_pc.light = p.light;

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
        },
      }, true };
      vee::cmd_set_viewport(cb, m_ext);
      vee::cmd_set_scissor(cb, m_ext);

      vee::cmd_bind_gr_pipeline(cb, *m_pln.ppl);
      vee::cmd_push_vertex_constants(cb, *m_clr.pl, &m_pc);
      vee::cmd_draw(cb, 4);

      vee::cmd_bind_gr_pipeline(cb, *m_clr.ppl);
      vee::cmd_bind_descriptor_set(cb, *m_clr.pl, 0, p.tmap);
      vee::cmd_bind_vertex_buffers(cb, 0, p.vtx, 0);
      vee::cmd_bind_vertex_buffers(cb, 1, p.inst, 0);
      vee::cmd_bind_index_buffer_u16(cb, p.idx);
      vee::cmd_draw_indexed(cb, {
        .xcount = 36,
        .icount = p.icount,
      });

      vee::cmd_bind_gr_pipeline(cb, *m_shd.ppl);
      vee::cmd_bind_vertex_buffers(cb, 0, p.shdvtx, 0);
      vee::cmd_bind_index_buffer_u16(cb, p.shdidx);
      vee::cmd_draw_indexed(cb, {
        .xcount = p.sicount,
        .icount = p.icount,
      });

      vee::cmd_bind_gr_pipeline(cb, *m_lig.ppl);
      vee::cmd_bind_vertex_buffers(cb, 0, p.vtx, 0);
      vee::cmd_bind_index_buffer_u16(cb, p.idx);
      vee::cmd_draw_indexed(cb, {
        .xcount = 36,
        .icount = p.icount,
      });
    }
  };
}
