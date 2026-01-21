#pragma leco add_shader "ofs-colour.frag"
#pragma leco add_shader "ofs-colour.vert"
#pragma leco add_shader "ofs-lights.frag"
#pragma leco add_shader "ofs-lights.vert"
#pragma leco add_shader "ofs-shadow.frag"
#pragma leco add_shader "ofs-shadow.vert"
export module ofs;
import :common;
import clay;
import cube;
import hai;
import texmap;
import traits;
import voo;

using namespace wagen;

namespace ofs::colour {
  export struct pipeline : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(
      *texmap::descriptor_set_layout(),
      vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline ppl = vee::create_graphics_pipeline({
      .pipeline_layout = *pl,
      .render_pass = *create_render_pass(max_sampling()),
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .multisampling = max_sampling(),
      .subpass = 0,
      .depth = vee::depth::op_less(),
      .blends {
        vee::colour_blend_classic(),
        vee::colour_blend_none(),
        vee::colour_blend_none(),
      },
      .shaders {
        *clay::vert_shader("ofs-colour", [] {}),
        *clay::frag_shader("ofs-colour", [] {}),
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
}

namespace ofs::shadow {
  export struct pipeline : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline ppl = vee::create_graphics_pipeline({
      .pipeline_layout = *pl,
      .render_pass = *create_render_pass(max_sampling()),
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .multisampling = max_sampling(),
      .back_face_cull = false,
      .subpass = 1,
      .depth = vee::depth::of({
        .depthTestEnable = vk_true,
        .depthWriteEnable = vk_false,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencilTestEnable = vk_true,
        .front = {
          .failOp = VK_STENCIL_OP_KEEP,
          .passOp = VK_STENCIL_OP_INCREMENT_AND_WRAP,
          .depthFailOp = VK_STENCIL_OP_KEEP,
          .compareOp = VK_COMPARE_OP_ALWAYS,
          .compareMask = ~0U,
          .writeMask = ~0U,
        },
        .back = {
          .failOp = VK_STENCIL_OP_KEEP,
          .passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP,
          .depthFailOp = VK_STENCIL_OP_KEEP,
          .compareOp = VK_COMPARE_OP_ALWAYS,
          .compareMask = ~0U,
          .writeMask = ~0U,
        },
      }),
      .blends {},
      .shaders {
        *clay::vert_shader("ofs-shadow", [] {}),
        *clay::frag_shader("ofs-shadow", [] {}),
      },
      .bindings {
        cube::v_buffer::vertex_input_bind(),
        cube::i_buffer::vertex_input_bind_per_instance(),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, traits::offset_of(&cube::vtx::pos)),
        vee::vertex_attribute_vec4(1, traits::offset_of(&cube::inst::pos)),
      },
    });
  };
}

namespace ofs::lights {
  export struct pipeline : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline ppl = vee::create_graphics_pipeline({
      .pipeline_layout = *pl,
      .render_pass = *create_render_pass(max_sampling()),
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .multisampling = max_sampling(),
      .subpass = 2,
      .depth = vee::depth::of({
        .depthTestEnable = vk_true,
        .depthWriteEnable = vk_false,
        .depthCompareOp = VK_COMPARE_OP_EQUAL,
        .stencilTestEnable = vk_true,
        .front = {
          .failOp = VK_STENCIL_OP_KEEP,
          .passOp = VK_STENCIL_OP_KEEP,
          .depthFailOp = VK_STENCIL_OP_KEEP,
          .compareOp = VK_COMPARE_OP_EQUAL,
          .compareMask = ~0U,
          .writeMask = ~0U,
        },
        .back = {
          .failOp = VK_STENCIL_OP_KEEP,
          .passOp = VK_STENCIL_OP_KEEP,
          .depthFailOp = VK_STENCIL_OP_KEEP,
          .compareOp = VK_COMPARE_OP_EQUAL,
          .compareMask = ~0U,
          .writeMask = ~0U,
        },
      }),
      .blends { vee::colour_blend_classic() },
      .shaders {
        *clay::vert_shader("ofs-lights", [] {}),
        *clay::frag_shader("ofs-lights", [] {}),
      },
      .bindings {
        cube::v_buffer::vertex_input_bind(),
        cube::i_buffer::vertex_input_bind_per_instance(),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, traits::offset_of(&cube::vtx::pos)),
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
    vee::buffer::type shdidx;
    unsigned icount;

    vee::descriptor_set tmap;
    float sun_angle;
  };

  export class pipeline {
    colour::pipeline m_clr {};
    shadow::pipeline m_shd {};
    lights::pipeline m_lig {};

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
      m_pc.sun_angle = p.sun_angle;

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

      vee::cmd_bind_gr_pipeline(cb, *m_clr.ppl);
      vee::cmd_bind_descriptor_set(cb, *m_clr.pl, 0, p.tmap);
      vee::cmd_push_vertex_constants(cb, *m_clr.pl, &m_pc);
      vee::cmd_bind_vertex_buffers(cb, 0, p.vtx, 0);
      vee::cmd_bind_vertex_buffers(cb, 1, p.inst, 0);
      vee::cmd_bind_index_buffer_u16(cb, p.idx);
      vee::cmd_draw_indexed(cb, {
        .xcount = 36,
        .icount = p.icount,
      });

      vee::cmd_next_subpass(cb);
      vee::cmd_bind_gr_pipeline(cb, *m_shd.ppl);
      vee::cmd_bind_index_buffer_u16(cb, p.shdidx);
      vee::cmd_draw_indexed(cb, {
        .xcount = 36 * 3,
        .icount = p.icount,
      });

      vee::cmd_next_subpass(cb);
      vee::cmd_bind_gr_pipeline(cb, *m_lig.ppl);
      vee::cmd_bind_index_buffer_u16(cb, p.idx);
      vee::cmd_draw_indexed(cb, {
        .xcount = 36,
        .icount = p.icount,
      });
    }
  };
}
