export module cube;
import dotz;
import clay;
import no;
import traits;
import voo;

using namespace traits::ints;

constexpr dotz::vec4 vtxes[] {
  {  0.5,  0.5,  0.5, 1.0 },
  {  0.5,  0.5, -0.5, 1.0 },
  {  0.5, -0.5,  0.5, 1.0 },
  {  0.5, -0.5, -0.5, 1.0 },
  { -0.5,  0.5,  0.5, 1.0 },
  { -0.5,  0.5, -0.5, 1.0 },
  { -0.5, -0.5,  0.5, 1.0 },
  { -0.5, -0.5, -0.5, 1.0 },
};

export namespace cube {
  struct vtx {
    dotz::vec4 pos;
    dotz::vec2 uv;
    dotz::vec3 normal;
  };
  struct inst {
    dotz::vec3 pos;
    float txtid;
  };
  struct v_buffer : public clay::buffer<vtx>, no::no {
    v_buffer() : clay::buffer<vtx> { 37 } {
      auto m = map();

      // Front
      m += vtx { .pos = vtxes[6], .uv { 1, 0 }, .normal { 0, 0, 1 } };
      m += vtx { .pos = vtxes[2], .uv { 0, 0 }, .normal { 0, 0, 1 } };
      m += vtx { .pos = vtxes[4], .uv { 1, 1 }, .normal { 0, 0, 1 } };
      m += vtx { .pos = vtxes[0], .uv { 0, 1 }, .normal { 0, 0, 1 } };

      // Back
      m += vtx { .pos = vtxes[7], .uv { 0, 0 }, .normal { 0, 0, -1 } };
      m += vtx { .pos = vtxes[5], .uv { 0, 1 }, .normal { 0, 0, -1 } };
      m += vtx { .pos = vtxes[3], .uv { 1, 0 }, .normal { 0, 0, -1 } };
      m += vtx { .pos = vtxes[1], .uv { 1, 1 }, .normal { 0, 0, -1 } };

      // Bottom
      m += vtx { .pos = vtxes[7], .uv { 1, 1 }, .normal { 0, -1, 0 } };
      m += vtx { .pos = vtxes[3], .uv { 0, 1 }, .normal { 0, -1, 0 } };
      m += vtx { .pos = vtxes[6], .uv { 1, 0 }, .normal { 0, -1, 0 } };
      m += vtx { .pos = vtxes[2], .uv { 0, 0 }, .normal { 0, -1, 0 } };

      // Top
      m += vtx { .pos = vtxes[5], .uv { 1, 1 }, .normal { 0, 1, 0 } };
      m += vtx { .pos = vtxes[4], .uv { 1, 0 }, .normal { 0, 1, 0 } };
      m += vtx { .pos = vtxes[1], .uv { 0, 1 }, .normal { 0, 1, 0 } };
      m += vtx { .pos = vtxes[0], .uv { 0, 0 }, .normal { 0, 1, 0 } };

      // Left
      m += vtx { .pos = vtxes[7], .uv { 0, 0 }, .normal { -1, 0, 0 } };
      m += vtx { .pos = vtxes[6], .uv { 1, 0 }, .normal { -1, 0, 0 } };
      m += vtx { .pos = vtxes[5], .uv { 0, 1 }, .normal { -1, 0, 0 } };
      m += vtx { .pos = vtxes[4], .uv { 1, 1 }, .normal { -1, 0, 0 } };

      // Right
      m += vtx { .pos = vtxes[3], .uv { 1, 0 }, .normal { 1, 0, 0 } };
      m += vtx { .pos = vtxes[1], .uv { 1, 1 }, .normal { 1, 0, 0 } };
      m += vtx { .pos = vtxes[2], .uv { 0, 0 }, .normal { 1, 0, 0 } };
      m += vtx { .pos = vtxes[0], .uv { 0, 1 }, .normal { 1, 0, 0 } };
    }
  };

  auto ix_buffer() {
    auto bb = voo::bound_buffer::create_from_host(sizeof(uint16_t) * 36, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    struct tri { uint16_t x[3]; };
    voo::memiter<tri> m { *bb.memory };
    
    m += {{  0,  1,  2 }}; m += {{  3,  2,  1 }}; // Front
    m += {{  4,  5,  6 }}; m += {{  7,  6,  5 }}; // Back
    m += {{  8,  9, 10 }}; m += {{ 11, 10,  9 }}; // Bottom
    m += {{ 12, 13, 14 }}; m += {{ 15, 14, 13 }}; // Top
    m += {{ 16, 17, 18 }}; m += {{ 19, 18, 17 }}; // Left
    m += {{ 20, 21, 22 }}; m += {{ 23, 22, 21 }}; // Right

    return bb;
  }

  struct shadow_v_buffer : public clay::buffer<dotz::vec4>, no::no {
    shadow_v_buffer() : clay::buffer<dotz::vec4> { 8 } {
      auto m = map();
      m += {};
      for (auto v : vtxes) m += v;
    }
  };
  class shadow_ix_buffer {
    voo::bound_buffer m_bb;
    unsigned m_count = 0;

  public:
    shadow_ix_buffer() :
      m_bb { voo::bound_buffer::create_from_host(sizeof(uint16_t) * 36 * 3, VK_BUFFER_USAGE_INDEX_BUFFER_BIT) }
    {}

    void setup(dotz::vec3 l) {
      struct tri { uint16_t x[3]; };
      voo::memiter<tri> m { *m_bb.memory, &m_count };
      
      const auto mm = [&](uint16_t a, uint16_t b, uint16_t c) {
        m += {{ a, b, c }};
      };
      const auto cap = [&](dotz::vec3 normal, uint16_t a, uint16_t b, uint16_t c, uint16_t d) {
        bool backface = (dotz::dot(normal, -l) < 0);
        if (backface) return;
        mm(a + 1, b + 1, c + 1);
        mm(d + 1, c + 1, b + 1);
      };
      cap({  0,  0,  1 }, 6, 2, 4, 0);
      cap({  0,  0, -1 }, 7, 5, 3, 1);
      cap({  0, -1,  0 }, 7, 3, 6, 2);
      cap({  0,  1,  0 }, 5, 4, 1, 0);
      cap({ -1,  0,  0 }, 7, 6, 5, 4);
      cap({  1,  0,  0 }, 3, 1, 2, 0);
    }

    [[nodiscard]] constexpr auto operator*() const { return *m_bb.buffer; }
    [[nodiscard]] constexpr auto count() const { return m_count * 3; }
  };

  struct i_buffer : clay::buffer<inst>, no::no {
    i_buffer() : clay::buffer<inst> { 128 * 128 * 2} {
      auto m = map();
      for (auto x = 0; x < 128; x++) {
        for (auto y = 0; y < 128; y++) {
          unsigned n = (x + y) % 4;
          if (n == 3) continue;

          m += inst {
            .pos { x - 64, -1, y - 64 },
            .txtid = static_cast<float>(n),
          };
        }
      }
      m += inst {
        .pos { 3, 0, 5 },
        .txtid = static_cast<float>(0),
      };
    }
  };
}

inline auto win32_hack() {
  // Pre-instantiate the simple to avoid clang exploding later
  return traits::offset_of(&cube::vtx::pos) + traits::offset_of(&cube::inst::pos);
}

