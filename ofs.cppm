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
    dotz::vec4 rot { 0, 0, 0, 1 };
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

  template<unsigned N> consteval unsigned size1(const auto (&)[N]) { return N; }
  consteval unsigned size(auto &... as) { return (size1(as) + ...); }

  export class v_buffer : public clay::buffer<vtx>, no::no {
    template<typename T>
    void push(auto & m, T) {
      for (auto v : T::vtx) m += { .pos = T::pos[v.id], .uv = v.uv, .normal = v.normal };
    }

  public:
    template<typename... T> v_buffer(T...) : buffer { size(T::pos...) } {
      auto m = map();
      (push(m, T {}), ...);
    }
  };

  export class ix_buffer : public clay::ix_buffer<uint16_t> {
    template<typename T>
    void push(auto & m, T) {
      for (auto [a, b, c] : T::tri) {
        m += a; m += b; m += c;
      };
    }

  public:
    template<typename... T> ix_buffer(T...) : 
      clay::ix_buffer<uint16_t> { size(T::tri...) * 3 }
    {
      auto m = map();
      (push(m, T {}), ...);
    }
  };

  export class e_buffer : public clay::buffer<edge>, no::no {
    template<typename T> void push(auto & map, T) {
      for (auto [m, n] : T::edg) {
        ofs::edge e {
          .vtx_a = T::pos[m],
          .vtx_b = T::pos[n],
        };
        for (auto [ia, ib, ic] : T::tri) {
          auto va = T::vtx[ia];
          auto vb = T::vtx[ib];
          auto vc = T::vtx[ic];

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
  public:
    template<typename... T>
    e_buffer(T...) : clay::buffer<edge> { size(T::edg...) } {
      auto m = map();
      (push(m, T {}), ...);
    }
  };
  export struct i_buffer : clay::buffer<ofs::inst>, no::no {
    using clay::buffer<ofs::inst>::buffer;
  };

  export struct drawer {
    virtual void faces(VkCommandBuffer cb, VkPipelineLayout pl) = 0;
    virtual void edges(VkCommandBuffer cb) = 0;
  };
  export struct buffers : drawer {
    v_buffer vtx;
    ix_buffer idx;
    e_buffer edg;
    i_buffer ins;

    template<typename T>
    explicit buffers(T, unsigned i_count) :
      vtx { T {} }
    , idx { T {} }
    , edg { T {} }
    , ins { i_count }
    {}

    [[nodiscard]] auto map() { return ins.map(); }

    void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
      if (ins.count() == 0) return;
      vee::cmd_bind_vertex_buffers(cb, 0, *vtx, 0);
      vee::cmd_bind_vertex_buffers(cb, 1, *ins, 0);
      vee::cmd_bind_index_buffer_u16(cb, *idx);
      vee::cmd_draw_indexed(cb, {
        .xcount = idx.count(),
        .icount = ins.count(),
      });
    }
    void edges(vee::command_buffer cb) override {
      if (ins.count() == 0) return;
      vee::cmd_bind_vertex_buffers(cb, 0, *edg, 0);
      vee::cmd_bind_vertex_buffers(cb, 1, *ins, 0);
      vee::cmd_draw(cb, {
        .vcount = edg.count(),
        .icount = ins.count(),
      });
    }
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
        vee::vertex_attribute_vec4(1, traits::offset_of(&inst::rot)),
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
        vee::vertex_attribute_vec4(1, traits::offset_of(&inst::rot)),
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
        vee::vertex_attribute_vec4(1, traits::offset_of(&inst::rot)),
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
        vee::vertex_attribute_vec4(1, traits::offset_of(&inst::rot)),
      },
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
