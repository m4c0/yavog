export module cube;
import dotz;
import clay;
import no;
import traits;
import voo;

using namespace traits::ints;

constexpr dotz::vec4 vtxes[] {
  {  0.5,  0.5,  0.5, 1.0 }, // 0
  {  0.5,  0.5, -0.5, 1.0 }, // 1
  {  0.5, -0.5,  0.5, 1.0 }, // 2
  {  0.5, -0.5, -0.5, 1.0 }, // 3

  { -0.5,  0.5,  0.5, 1.0 }, // 4
  { -0.5,  0.5, -0.5, 1.0 }, // 5
  { -0.5, -0.5,  0.5, 1.0 }, // 6
  { -0.5, -0.5, -0.5, 1.0 }, // 7

  { 0, 0, 0, 0 }, // Light projection at infinity
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

  struct shadow_edge {
    dotz::vec4 nrm_a;
    dotz::vec4 nrm_b;
    dotz::vec4 vtx_a;
    dotz::vec4 vtx_b;
  };
  struct shadow_v_buffer : public clay::buffer<shadow_edge>, no::no {
    shadow_v_buffer() : clay::buffer<shadow_edge> { 13 } {
      auto m = map();

      m += { {  0,  0,  1 }, { -1,  0,  0 }, vtxes[6], vtxes[4] };
      m += { {  0,  0,  1 }, {  1,  0,  0 }, vtxes[0], vtxes[2] };
      m += { {  0,  0,  1 }, {  0, -1,  0 }, vtxes[2], vtxes[6] };
      m += { {  0,  0,  1 }, {  0,  1,  0 }, vtxes[4], vtxes[0] };

      m += { {  0,  0, -1 }, { -1,  0,  0 }, vtxes[5], vtxes[7] };
      m += { {  0,  0, -1 }, {  1,  0,  0 }, vtxes[3], vtxes[1] };
      m += { {  0,  0, -1 }, {  0, -1,  0 }, vtxes[7], vtxes[3] };
      m += { {  0,  0, -1 }, {  0,  1,  0 }, vtxes[1], vtxes[5] };

      m += { {  0, -1,  0 }, { -1,  0,  0 }, vtxes[7], vtxes[6] };
      m += { {  0, -1,  0 }, {  1,  0,  0 }, vtxes[2], vtxes[3] };

      m += { {  0,  1,  0 }, { -1,  0,  0 }, vtxes[4], vtxes[5] };
      m += { {  0,  1,  0 }, {  1,  0,  0 }, vtxes[1], vtxes[0] };
      
      m += {};
    }
  };
  struct shadow_ix_buffer : public clay::ix_buffer<uint16_t>, no::no {
    shadow_ix_buffer() : clay::ix_buffer<uint16_t> { 36 } {
      auto m = map();
      for (uint16_t i = 0; i < 12; i++) {
        m += i; m += i; m += 12;
      }
    }
  };

  /*
    void setup(dotz::vec3 l) {
      const auto backface = [&](dotz::vec3 n) { return (dotz::dot(n, l) >= 0); };
      const auto side = [&](dotz::vec3 n1, dotz::vec3 n2, uint16_t a, uint16_t b) {
        bool b1 = backface(n1);
        bool b2 = backface(n2);
        if (b1 == b2) return;

        if (!b1 && b2) {
          mm(a, b, 9);
        } else {
          mm(b, a, 9);
        }
      };

    */

  struct i_buffer : clay::buffer<inst>, no::no {
    i_buffer() : clay::buffer<inst> { 128 * 128 * 2} {
      auto m = map();
      for (auto x = 0; x < 128; x++) {
        for (auto y = 0; y < 128; y++) {
          unsigned n = (x + y) % 4;
          if (n == 3) continue;

          m += inst {
            .pos { x - 64, -2, y },
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
  return traits::offset_of(&cube::vtx::pos) + traits::offset_of(&cube::inst::pos) + traits::offset_of(&cube::shadow_edge::nrm_a);
}

