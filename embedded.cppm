export module embedded;
import buffers;
import hai;
import sv;
import texmap;
import voo;

using namespace wagen;

namespace embedded {
  struct model_cmd {
    unsigned id;
    unsigned i_count;
    unsigned first_i;
    int v_offset;
    unsigned v_count;
    unsigned e_count;
    unsigned first_e;

    template<typename T> static model_cmd of(T) {
      return {
        .id = T::id,

        .i_count = buffers::size(T::tri) * 3,
        .v_count = buffers::size(T::vtx),
        .e_count = buffers::size(T::edg) * 3,
      };
    }
  };

  class model_cmds {
    hai::varray<model_cmd> m_mdls;
    model_cmd m_last {};

    template<typename T> void cmd(T) {
      auto m = model_cmd::of(T {});

      m.first_i  = m_last.first_i  + m_last.i_count;
      m.v_offset = m_last.v_offset + m_last.v_count;
      m.first_e  = m_last.first_e  + m_last.e_count;

      m_last = m;
      m_mdls.push_back(m);
    }

  public:
    template<typename... T>
      model_cmds(T...) : m_mdls { sizeof...(T) } {
        (cmd(T {}), ...);
      }

    template<typename T>
      [[nodiscard]] constexpr model_cmd operator[](T) const {
        for (auto m : m_mdls) if (m.id == T::id) return m;
        return {};
      }
  };
  export class drawer {
    texmap::cache m_tmap {};
    buffers::v_buffer  m_vtx;
    buffers::ix_buffer m_idx;
    buffers::e_buffer  m_edg;
    model_cmds m_mdls;

    buffers::buffer<VkDrawIndexedIndirectCommand> m_vcmd;
    buffers::buffer<VkDrawIndirectCommand> m_ecmd;

    buffers::buffer<buffers::inst> m_ins;

    friend class builder;

  public:
    template<typename... T> drawer(unsigned max_ins, T...) :
      m_vtx  { T {}... }
    , m_idx  { T {}... }
    , m_edg  { T {}... }
    , m_mdls { T {}... }
    , m_vcmd { sizeof...(T) + 1 }
    , m_ecmd { sizeof...(T) + 1 }
    , m_ins  { max_ins }
    {
      auto vc = m_vcmd.map();
      auto ec = m_ecmd.map();
      vc += {}; ec += {}; // "none"

      const model_cmds mdls { T {}... };
      const auto push = [&](model_cmd m) {
        vc += {
          .indexCount = m.i_count,
          .firstIndex = m.first_i,
          .vertexOffset = m.v_offset,
        };
        ec += {
          .vertexCount = m.e_count,
          .firstVertex = m.first_e,
        };
      };
      (push(mdls[T()]), ...);
    }

    [[nodiscard]] auto model(auto t) { return m_mdls[t]; }
    [[nodiscard]] auto texture(sv name) { return m_tmap.load(name); }

    constexpr auto vcmd() const { return *m_vcmd; }
    constexpr auto ecmd() const { return *m_ecmd; }

    void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) {
      if (pl) vee::cmd_bind_descriptor_set(cb, pl, 0, m_tmap.dset());

      vee::cmd_bind_vertex_buffers(cb, 0, *m_vtx, 0);
      vee::cmd_bind_index_buffer_u16(cb, *m_idx);
    }
    void edges(vee::command_buffer cb) {
      vee::cmd_bind_vertex_buffers(cb, 0, *m_edg, 0);
    }
  };

  export class builder {
    voo::memiter<VkDrawIndexedIndirectCommand> m_vc;
    voo::memiter<VkDrawIndirectCommand> m_ec;
    voo::memiter<buffers::inst> m_inst;
    unsigned m_count {};

  public:
    explicit builder(drawer & d) :
      m_vc { d.m_vcmd.map() }
    , m_ec { d.m_ecmd.map() }
    , m_inst { d.m_ins.map() }
    {}

    void push(model_cmd m) {
      m_vc += {
        .indexCount = m.i_count,
        .instanceCount = m_inst.count() - m_count,
        .firstIndex = m.first_i,
        .vertexOffset = m.v_offset,
        .firstInstance = m_count,
      };
      m_ec += {
        .vertexCount = m.e_count,
        .instanceCount = m_inst.count() - m_count,
        .firstVertex = m.first_e,
        .firstInstance = m_count,
      };

      m_count = m_inst.count();
    }

    void operator+=(buffers::inst i) { m_inst += i; }
  };
}
