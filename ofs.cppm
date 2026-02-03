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
import hai;
import models;
import texmap;
import traits;
import voo;

using namespace traits::ints;
using namespace wagen;

namespace ofs {
  export struct vtx {
    dotz::vec4 pos;
    dotz::vec2 uv;
    dotz::vec3 normal;
  };
  export struct inst {
    dotz::vec3 pos;
    float txtid;
  };
  export struct edge {
    dotz::vec4 nrm_a;
    dotz::vec4 nrm_b;
    dotz::vec4 vtx_a;
    dotz::vec4 vtx_b;
  };

  export struct upc {
    dotz::vec4 light;
    float aspect;
    float fov = 90;
    float far;
  };

  export struct v_buffer : public clay::buffer<vtx>, no::no {
    template<unsigned N, unsigned M>
    v_buffer(const models::vtx (&vtx)[N], const dotz::vec4 (&pos)[M]) : buffer { N } {
      auto m = map();
      for (auto v : vtx) m += { .pos = pos[v.id], .uv = v.uv, .normal = v.normal };
    }
  };
  export struct ix_buffer : clay::ix_buffer<uint16_t> {
    template<unsigned N>
    ix_buffer(const models::tri (&tri)[N]) : clay::ix_buffer<uint16_t> { N * 3 } {
      auto m = map();
      for (auto [a, b, c] : tri) {
        m += a; m += b; m += c;
      };
    }
  };
  export struct e_buffer : clay::buffer<edge>, no::no {
    template<unsigned N>
    e_buffer(const auto & t, const models::edge (&edg)[N]) : clay::buffer<edge> { N } {
      auto map = this->map();
      for (auto [m, n] : edg) {
        ofs::edge e {
          .vtx_a = t.pos[m],
          .vtx_b = t.pos[n],
        };
        for (auto [ia, ib, ic] : t.tri) {
          auto va = t.vtx[ia];
          auto vb = t.vtx[ib];
          auto vc = t.vtx[ic];

          if (va.id == m && vb.id == n) {
            e.nrm_b = { va.normal, 0 };
          } else if (vb.id == m && vc.id == n) {
            e.nrm_b = { va.normal, 0 };
          } else if (vc.id == m && va.id == n) {
            e.nrm_b = { va.normal, 0 };
          } else if (va.id == n && vb.id == m) {
            e.nrm_a = { vb.normal, 0 };
          } else if (vb.id == n && vc.id == m) {
            e.nrm_a = { vb.normal, 0 };
          } else if (vc.id == n && va.id == m) {
            e.nrm_a = { vb.normal, 0 };
          }
        }
        map += {};
        e.nrm_a.w = 1; map += e;
        e.nrm_a.w = 2; map += e;
      }
    }
  };
  export struct buffers {
    v_buffer vtx;
    ix_buffer idx;
    e_buffer edg;

    template<typename T>
    explicit buffers(T) :
      vtx { T::vtx, T::pos }
    , idx { T::tri }
    , edg { T {}, T::edg }
    {}
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
        vee::vertex_input_bind(sizeof(vtx)),
        vee::vertex_input_bind_per_instance(sizeof(inst)),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, traits::offset_of(&vtx::pos)),
        vee::vertex_attribute_vec2(0, traits::offset_of(&vtx::uv)),
        vee::vertex_attribute_vec3(0, traits::offset_of(&vtx::normal)),
        vee::vertex_attribute_vec4(1, traits::offset_of(&inst::pos)),
      },
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
      .bindings {
        vee::vertex_input_bind(sizeof(edge)),
        vee::vertex_input_bind_per_instance(sizeof(inst)),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, traits::offset_of(&edge::nrm_a)),
        vee::vertex_attribute_vec4(0, traits::offset_of(&edge::nrm_b)),
        vee::vertex_attribute_vec4(0, traits::offset_of(&edge::vtx_a)),
        vee::vertex_attribute_vec4(0, traits::offset_of(&edge::vtx_b)),
        vee::vertex_attribute_vec4(1, traits::offset_of(&inst::pos)),
      },
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
      .bindings {
        vee::vertex_input_bind(sizeof(vtx)),
        vee::vertex_input_bind_per_instance(sizeof(inst)),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, traits::offset_of(&vtx::pos)),
        vee::vertex_attribute_vec3(0, traits::offset_of(&vtx::normal)),
        vee::vertex_attribute_vec4(1, traits::offset_of(&inst::pos)),
      },
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
      .bindings {
        vee::vertex_input_bind(sizeof(vtx)),
        vee::vertex_input_bind_per_instance(sizeof(inst)),
      },
      .attributes { 
        vee::vertex_attribute_vec4(0, traits::offset_of(&vtx::pos)),
        vee::vertex_attribute_vec2(0, traits::offset_of(&vtx::uv)),
        vee::vertex_attribute_vec3(0, traits::offset_of(&vtx::normal)),
        vee::vertex_attribute_vec4(1, traits::offset_of(&inst::pos)),
      },
    });

    lights() {
      vee::set_debug_utils_object_name(*pl, "ofs::lights");
      vee::set_debug_utils_object_name(*ppl, "ofs::lights");
    }
  };
}

namespace ofs {
  export struct drawer {
    virtual void faces(VkCommandBuffer cb, VkPipelineLayout pl) = 0;
    virtual void edges(VkCommandBuffer cb) = 0;
  };
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
