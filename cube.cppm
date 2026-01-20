export module cube;
import dotz;
import clay;
import no;
import traits;
import voo;

using namespace traits::ints;

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

      m += vtx {};

      // Front
      m += vtx { .pos { -0.5, -0.5,  0.5, 1.0 }, .uv { 1, 0 }, .normal { 0, 0, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5, 1.0 }, .uv { 0, 0 }, .normal { 0, 0, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5, 1.0 }, .uv { 1, 1 }, .normal { 0, 0, 1 } };
      m += vtx { .pos {  0.5,  0.5,  0.5, 1.0 }, .uv { 0, 1 }, .normal { 0, 0, 1 } };

      // Back
      m += vtx { .pos { -0.5, -0.5, -0.5, 1.0 }, .uv { 0, 0 }, .normal { 0, 0, -1 } };
      m += vtx { .pos { -0.5,  0.5, -0.5, 1.0 }, .uv { 0, 1 }, .normal { 0, 0, -1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5, 1.0 }, .uv { 1, 0 }, .normal { 0, 0, -1 } };
      m += vtx { .pos {  0.5,  0.5, -0.5, 1.0 }, .uv { 1, 1 }, .normal { 0, 0, -1 } };

      // Bottom
      m += vtx { .pos { -0.5, -0.5, -0.5, 1.0 }, .uv { 1, 1 }, .normal { 0, -1, 0 } };
      m += vtx { .pos {  0.5, -0.5, -0.5, 1.0 }, .uv { 0, 1 }, .normal { 0, -1, 0 } };
      m += vtx { .pos { -0.5, -0.5,  0.5, 1.0 }, .uv { 1, 0 }, .normal { 0, -1, 0 } };
      m += vtx { .pos {  0.5, -0.5,  0.5, 1.0 }, .uv { 0, 0 }, .normal { 0, -1, 0 } };

      // Top
      m += vtx { .pos { -0.5,  0.5, -0.5, 1.0 }, .uv { 1, 1 }, .normal { 0, 1, 0 } };
      m += vtx { .pos { -0.5,  0.5,  0.5, 1.0 }, .uv { 1, 0 }, .normal { 0, 1, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5, 1.0 }, .uv { 0, 1 }, .normal { 0, 1, 0 } };
      m += vtx { .pos {  0.5,  0.5,  0.5, 1.0 }, .uv { 0, 0 }, .normal { 0, 1, 0 } };

      // Left
      m += vtx { .pos { -0.5, -0.5, -0.5, 1.0 }, .uv { 0, 0 }, .normal { -1, 0, 0 } };
      m += vtx { .pos { -0.5, -0.5,  0.5, 1.0 }, .uv { 1, 0 }, .normal { -1, 0, 0 } };
      m += vtx { .pos { -0.5,  0.5, -0.5, 1.0 }, .uv { 0, 1 }, .normal { -1, 0, 0 } };
      m += vtx { .pos { -0.5,  0.5,  0.5, 1.0 }, .uv { 1, 1 }, .normal { -1, 0, 0 } };

      // Right
      m += vtx { .pos {  0.5, -0.5, -0.5, 1.0 }, .uv { 1, 0 }, .normal { 1, 0, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5, 1.0 }, .uv { 1, 1 }, .normal { 1, 0, 0 } };
      m += vtx { .pos {  0.5, -0.5,  0.5, 1.0 }, .uv { 0, 0 }, .normal { 1, 0, 0 } };
      m += vtx { .pos {  0.5,  0.5,  0.5, 1.0 }, .uv { 0, 1 }, .normal { 1, 0, 0 } };
    }
  };

  auto ix_buffer() {
    auto bb = voo::bound_buffer::create_from_host(sizeof(uint16_t) * 36, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    struct tri { uint16_t x[3]; };
    voo::memiter<tri> m { *bb.memory };
    
    m += {{  1,  2,  3 }}; m += {{  4,  3,  2 }}; // Front
    m += {{  5,  6,  7 }}; m += {{  8,  7,  6 }}; // Back
    m += {{  9, 10, 11 }}; m += {{ 12, 11, 10 }}; // Bottom
    m += {{ 13, 14, 15 }}; m += {{ 16, 15, 14 }}; // Top
    m += {{ 17, 18, 19 }}; m += {{ 20, 19, 18 }}; // Left
    m += {{ 21, 22, 23 }}; m += {{ 24, 23, 22 }}; // Right

    return bb;
  }

  class shadow_ix_buffer {
    voo::bound_buffer m_bb;
    unsigned m_count;

  public:
    shadow_ix_buffer() :
      m_bb { voo::bound_buffer::create_from_host(sizeof(uint16_t) * 36, VK_BUFFER_USAGE_INDEX_BUFFER_BIT) }
    {}

    void setup(dotz::vec3 l) {
      struct tri { uint16_t x[3]; };
      voo::memiter<tri> m { *m_bb.memory, &m_count };
      
      const auto add = [&](uint16_t a, uint16_t b, dotz::vec3 normal) {
        // backface
        if (dotz::dot(normal, l) < 0) m += {{ a, b, 0 }};
        else                          m += {{ b, a, 0 }};
      };
      add(1, 2, { 0, 0, 1 });
    }

    [[nodiscard]] constexpr auto count() const { return m_count; }
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
