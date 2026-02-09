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
    dotz::vec4 pos; // pos(3) + txtid
    dotz::vec4 rot { 0, 0, 0, 1 };
    dotz::vec2 size { 1, 1 };
  };
  export struct edge {
    dotz::vec4 nrm_a;
    dotz::vec4 nrm_b;
    dotz::vec4 vtx_a;
    dotz::vec4 vtx_b;
  };

  export template<unsigned N> consteval unsigned size1(const auto (&)[N]) { return N; }
  export consteval unsigned size(auto &... as) { return (size1(as) + ...); }

  template<typename T> VkBufferUsageFlagBits usage() {
    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }
  template<> VkBufferUsageFlagBits usage<uint16_t>() {
    return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }
  template<> VkBufferUsageFlagBits usage<VkDrawIndexedIndirectCommand>() {
    return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  }
  template<> VkBufferUsageFlagBits usage<VkDrawIndirectCommand>() {
    return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
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

    [[nodiscard]] auto map() {
      return voo::memiter<T> { *m_buf.memory, &m_count };
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
  };
  export struct i_buffer : buffer<inst> {
    using buffer<inst>::buffer;
  };
}

// Should be in this module due to a bug in clang on windows
export namespace buffers::vk {
  template<typename T> auto binding() { return vee::vertex_input_bind(sizeof(T)); }
  template<> auto binding<buffers::inst>() { return vee::vertex_input_bind_per_instance(sizeof(buffers::inst)); }
  template<typename... T> auto bindings() {
    return hai::view<VkVertexInputBindingDescription> { binding<T>()... };
  }

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
  auto attr(auto (buffers::inst::*m)) { return attr(1, m); }
  auto attr(auto (buffers::edge::*m)) { return attr(0, m); }

  struct attrs : hai::view<VkVertexInputAttributeDescription> {
    explicit attrs(auto... ms) : view { attr(ms)... } {}
  };
}

