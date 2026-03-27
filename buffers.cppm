export module buffers;
import dotz;
import hai;
import no;
import traits;
import voo;

using namespace traits::ints;
using namespace wagen;

namespace buffers {
  export struct vtx {
    dotz::vec4 pos;
    dotz::vec2 uv;
    dotz::vec3 normal;
  };
  export struct inst {
    dotz::vec4 rot { 0, 0, 0, 1 };
    dotz::vec3 pos {};
    float mdl;
    dotz::vec3 size { 1 };
    float txtid;
  };
  export struct edge {
    dotz::vec4 nrm_a;
    dotz::vec4 nrm_b;
    dotz::vec4 vtx_a;
    dotz::vec4 vtx_b;
  };

  export template<unsigned N> consteval unsigned size1(const auto (&)[N]) { return N; }
  export consteval unsigned size(auto &... as) { return (size1(as) + ...); }

  template<typename T> VkBufferUsageFlags usage() {
    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }
  template<> VkBufferUsageFlags usage<inst>() {
    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  template<> VkBufferUsageFlags usage<uint16_t>() {
    return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }
  template<> VkBufferUsageFlags usage<VkDrawIndexedIndirectCommand>() {
    return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  template<> VkBufferUsageFlags usage<VkDrawIndirectCommand>() {
    return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  template<> VkBufferUsageFlags usage<VkDispatchIndirectCommand>() {
    return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }

  export template<typename T> class buffer : no::no {
    voo::bound_buffer m_buf;

    unsigned m_count {};
  public:
    explicit buffer(unsigned max) :
      m_buf { voo::bound_buffer::create_from_host(max * sizeof(T), usage<T>()) }
    {}

    [[nodiscard]] constexpr auto operator*() const { return *m_buf.buffer; }
    [[nodiscard]] constexpr auto count() const { return m_count; }

    [[nodiscard]] auto map(unsigned * c) {
      return voo::memiter<T> { *m_buf.memory, c };
    }
    [[nodiscard]] auto map() {
      return voo::memiter<T> { *m_buf.memory, &m_count };
    }
    [[nodiscard]] constexpr auto memory() const { return *m_buf.memory; }

    void update_buffer(VkCommandBuffer cb, const T & val, unsigned idx) {
      vee::cmd_update_buffer(cb, *m_buf.buffer, &val, idx * sizeof(T));
    }
  };

  export class i_buffer : public buffer<inst> {
  public:
    i_buffer(unsigned count) : buffer { count } {}

    void cmd_bind(vee::command_buffer cb) {
      vee::cmd_bind_vertex_buffers(cb, 1, **this, 0);
    }
  };

  export class v_buffer : public buffer<vtx> {
    template<typename T>
    void push(auto & m, T) {
      for (auto v : T::vtx) m += { .pos = T::pos[v.id], .uv = v.uv, .normal = v.normal };
    }

  public:
    template<typename... T> v_buffer(T...) : buffer { size(T::vtx...) } {
      auto m = map();
      (push(m, T {}), ...);
    }

    void cmd_bind(vee::command_buffer cb) {
      vee::cmd_bind_vertex_buffers(cb, 0, **this, 0);
    }
  };

  export class ix_buffer : public buffer<uint16_t> {
    template<typename T>
    void push(auto & m, T) {
      for (auto [a, b, c] : T::tri) {
        m += a; m += b; m += c;
      };
    }

  public:
    template<typename... T> ix_buffer(T...) : 
      buffer<uint16_t> { size(T::tri...) * 3 }
    {
      auto m = map();
      (push(m, T {}), ...);
    }

    void cmd_bind(vee::command_buffer cb) {
      vee::cmd_bind_index_buffer_u16(cb, **this);
    }
  };

  export class e_buffer : public buffer<edge> {
    template<typename T> void push(auto & map, T) {
      for (auto [m, n] : T::edg) {
        edge e {
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
    e_buffer(T...) : buffer<edge> { size(T::edg...) * 3 } {
      auto m = map();
      (push(m, T {}), ...);
    }

    void cmd_bind(vee::command_buffer cb) {
      vee::cmd_bind_vertex_buffers(cb, 0, **this, 0);
    }
  };
 
  export class vc_buffer : public buffer<VkDrawIndexedIndirectCommand> {
    template<typename T> void push(auto & map, unsigned & i, int & vofs, T) {
      map += {
        .indexCount   = size(T::tri) * 3,
        .firstIndex   = i,
        .vertexOffset = vofs,
      };
      i += size(T::tri) * 3;
      vofs += size(T::vtx);
    }

  public:
    template<typename... T> vc_buffer(T...) :
      buffer<VkDrawIndexedIndirectCommand> { sizeof...(T) + 1 }
    {
      unsigned i = 0;
      int vofs = 0;
      auto m = buffer::map();
      m += {};
      (push(m, i, vofs, T {}), ...);
    }

    void cmd_draw(vee::command_buffer cb) {
      for (auto i = 0; i < count(); i++) {
        vee::cmd_draw_indexed_indirect(cb, **this, i, 1);
      }
    }
  };

  export class ec_buffer : public buffer<VkDrawIndirectCommand> {
    template<typename T> void push(auto & map, unsigned & i, T) {
      map += {
        .vertexCount  = size(T::edg) * 3,
        .firstVertex  = i,
      };
      i += size(T::edg) * 3;
    }

  public:
    template<typename... T> ec_buffer(T...) :
      buffer<VkDrawIndirectCommand> { sizeof...(T) + 1 }
    {
      unsigned tmp = 0;
      auto m = map();
      m += {};
      (push(m, tmp, T {}), ...);
    }

    void cmd_draw(vee::command_buffer cb) {
      for (auto i = 0; i < count(); i++) {
        vee::cmd_draw_indirect(cb, **this, i, 1);
      }
    }
  };

  export class dc_buffer : public buffer<VkDispatchIndirectCommand> {
  public:
    dc_buffer() : buffer<VkDispatchIndirectCommand> { 1 } {
      auto m = map();
      m += { 1, 1, 1 };
    }

    void cmd_draw(vee::command_buffer cb) {
      vee::cmd_dispatch_indirect(cb, **this);
    }
  };

  export struct all {
    v_buffer  vtx;
    ix_buffer idx;
    e_buffer  edg;
    vc_buffer vcmd;
    ec_buffer ecmd;
    dc_buffer dcmd {};
    i_buffer  inst;

    template<typename... T> all(unsigned insts, T...) :
      vtx  { T {}... }
    , idx  { T {}... }
    , edg  { T {}... }
    , vcmd { T {}... }
    , ecmd { T {}... }
    , inst { insts }
    {}

    void cmd_draw_vtx(vee::command_buffer cb) {
      vtx.cmd_bind(cb);
      idx.cmd_bind(cb);
      inst.cmd_bind(cb);
      vcmd.cmd_draw(cb);
    }
    void cmd_draw_edg(vee::command_buffer cb) {
      edg.cmd_bind(cb);
      inst.cmd_bind(cb);
      ecmd.cmd_draw(cb);
    }
  };
}

// Should be in this module due to a bug in clang on windows
export namespace buffers::vk {
  template<typename T> auto binding() { return vee::vertex_input_bind(sizeof(T)); }
  template<> auto binding<buffers::inst>() { return vee::vertex_input_bind_per_instance(sizeof(buffers::inst)); }
  template<typename... T> auto bindings() {
    return hai::view<VkVertexInputBindingDescription> { binding<T>()... };
  }
  auto ibindings() { return bindings<vtx, inst>(); }

  template<typename T> auto attr(unsigned b, dotz::vec2 (T::*m)) {
    return vee::vertex_attribute_vec2(b, traits::offset_of(m));
  }
  template<typename T> auto attr(unsigned b, dotz::vec3 (T::*m)) {
    return vee::vertex_attribute_vec3(b, traits::offset_of(m));
  }
  template<typename T> auto attr(unsigned b, dotz::vec4 (T::*m)) {
    return vee::vertex_attribute_vec4(b, traits::offset_of(m));
  }
  auto attr(auto (buffers::vtx::*m)) { return attr(0, m); }
  auto attr(auto (buffers::edge::*m)) { return attr(0, m); }

  struct attrs : hai::view<VkVertexInputAttributeDescription> {
    explicit attrs(auto... ms) : view { attr(ms)...  } {}
  };
  struct iattrs : hai::view<VkVertexInputAttributeDescription> { 
    explicit iattrs(auto... ms) : view {
      vee::vertex_attribute_vec4(1, traits::offset_of(&inst::pos)),
      vee::vertex_attribute_vec4(1, traits::offset_of(&inst::rot)),
      vee::vertex_attribute_vec4(1, traits::offset_of(&inst::size)),
      attr(ms)...
    } {}
    explicit iattrs() : iattrs {
      &buffers::vtx::pos,
      &buffers::vtx::normal,
      &buffers::vtx::uv,
    } {}
  };

  struct drawer {
    virtual void faces(VkCommandBuffer cb, VkPipelineLayout pl) = 0;
    virtual void edges(VkCommandBuffer cb) = 0;
  };
}

