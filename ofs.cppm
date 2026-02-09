#pragma leco add_shader "ofs-colour.frag"
#pragma leco add_shader "ofs-colour.vert"
#pragma leco add_shader "ofs-lights.frag"
#pragma leco add_shader "ofs-lights.vert"
#pragma leco add_shader "ofs-planes.frag"
#pragma leco add_shader "ofs-planes.vert"
#pragma leco add_shader "ofs-shadow.frag"
#pragma leco add_shader "ofs-shadow.vert"
#pragma leco add_shader "ofs-shcaps.frag"
#pragma leco add_shader "ofs-shcaps.vert"
export module ofs;
import :common;
import buffers;
import hai;
import models;
import texmap;
import traits;
import voo;

namespace ofs {
  export struct upc {
    dotz::vec4 light;
    float aspect;
    float fov = 90;
    float far;
  };

  export struct drawer {
    virtual void faces(VkCommandBuffer cb, VkPipelineLayout pl) = 0;
    virtual void edges(VkCommandBuffer cb) = 0;
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
      .bindings = buffers::vk::bindings<buffers::vtx, buffers::inst>(),
      .attributes = buffers::vk::attrs(
        &buffers::vtx::pos,
        &buffers::vtx::uv,
        &buffers::vtx::normal,
        &buffers::inst::pos,
        &buffers::inst::rot,
        &buffers::inst::size
      ),
    });

    colour() {
      vee::set_debug_utils_object_name(*pl, "ofs::colour");
      vee::set_debug_utils_object_name(*ppl, "ofs::colour");
    }
  };
  struct shadow : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline ppl = create_colour_only_pipeline("ofs-shadow", {
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
      .bindings = buffers::vk::bindings<buffers::edge, buffers::inst>(),
      .attributes = buffers::vk::attrs(
        &buffers::edge::nrm_a,
        &buffers::edge::nrm_b,
        &buffers::edge::vtx_a,
        &buffers::edge::vtx_b,
        &buffers::inst::pos,
        &buffers::inst::rot,
        &buffers::inst::size
      ),
    });

    shadow() {
      vee::set_debug_utils_object_name(*pl, "ofs::shadow");
      vee::set_debug_utils_object_name(*ppl, "ofs::shadow");
    }
  };
  struct shcaps : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
    vee::gr_pipeline ppl = create_colour_only_pipeline("ofs-shcaps", {
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
      .bindings = buffers::vk::bindings<buffers::vtx, buffers::inst>(),
      .attributes = buffers::vk::attrs( 
        &buffers::vtx::pos,
        &buffers::vtx::normal,
        &buffers::inst::pos,
        &buffers::inst::rot,
        &buffers::inst::size
      ),
    });

    shcaps() {
      vee::set_debug_utils_object_name(*pl, "ofs::shcaps");
      vee::set_debug_utils_object_name(*ppl, "ofs::shcaps");
    }
  };
  struct lights : no::no {
    vee::pipeline_layout pl = vee::create_pipeline_layout(
      *texmap::descriptor_set_layout(),
      vee::vert_frag_push_constant_range<upc>());
    vee::gr_pipeline ppl = create_colour_only_pipeline("ofs-lights", {
      .pipeline_layout = *pl,
      .depth = vee::depth::of({
        .depthTestEnable = vk_true,
        .depthWriteEnable = vk_false,
        .depthCompareOp = VK_COMPARE_OP_EQUAL,
        .stencilTestEnable = vk_true,
        .front = stencil(VK_STENCIL_OP_KEEP, VK_COMPARE_OP_EQUAL),
        .back  = stencil(VK_STENCIL_OP_KEEP, VK_COMPARE_OP_EQUAL),
      }),
      .bindings = buffers::vk::bindings<buffers::vtx, buffers::inst>(),
      .attributes = buffers::vk::attrs(
        &buffers::vtx::pos,
        &buffers::vtx::uv,
        &buffers::vtx::normal,
        &buffers::inst::pos,
        &buffers::inst::rot,
        &buffers::inst::size
      ),
    });

    lights() {
      vee::set_debug_utils_object_name(*pl, "ofs::lights");
      vee::set_debug_utils_object_name(*ppl, "ofs::lights");
    }
  };
}

namespace ofs {
  export class pipeline {
    colour m_clr {};
    shadow m_shd {};
    shcaps m_scp {};
    lights m_lig {};

    vee::extent m_ext {};
    hai::uptr<framebuffer> m_fb {};

  public:
    [[nodiscard]] constexpr const auto & fb() const { return *m_fb; }

    void setup(const voo::swapchain & swc) {
      m_ext = swc.extent();
      m_fb.reset(new framebuffer { swc.extent() });
    }

    void render(vee::command_buffer cb, drawer * d, upc pc) {
      voo::cmd_render_pass rp { vee::render_pass_begin {
        .command_buffer = cb,
        .render_pass = *m_fb->rp,
        .framebuffer = *m_fb->fb,
        .extent = m_ext,
        .clear_colours { 
          vee::clear_colour({ 0, 0, 0, 1 }), 
          vee::clear_colour({ 0, 0, pc.far, 0 }), 
          vee::clear_colour({ 0, 0, 0, 0 }), 
          vee::clear_depth(1.0),
          vee::clear_colour({ 0, 0, 0, 1 }), 
          vee::clear_colour({ 0, 0, pc.far, 0 }), 
          vee::clear_colour({ 0, 0, 0, 0 }), 
        },
      }, true };
      vee::cmd_set_viewport(cb, m_ext);
      vee::cmd_set_scissor(cb, m_ext);

      vee::cmd_push_vert_frag_constants(cb, *m_lig.pl, &pc);

      vee::cmd_bind_gr_pipeline(cb, *m_clr.ppl);
      d->faces(cb, *m_clr.pl);

      vee::cmd_bind_gr_pipeline(cb, *m_scp.ppl);
      d->faces(cb, nullptr);

      vee::cmd_bind_gr_pipeline(cb, *m_shd.ppl);
      d->edges(cb);

      vee::cmd_bind_gr_pipeline(cb, *m_lig.ppl);
      d->faces(cb, *m_lig.pl);
    }
  };
}
