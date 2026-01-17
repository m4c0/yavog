export module cube;
import dotz;
import clay;
import no;

export namespace cube {
  struct vtx {
    dotz::vec3 pos;
    float _pad;
    dotz::vec2 uv;
    dotz::vec3 normal;
  };
  struct inst {
    dotz::vec3 pos;
    float txtid;
  };

  struct v_buffer : public clay::buffer<vtx>, no::no {
    v_buffer() : clay::buffer<vtx> { 36 } {
      auto m = map();

      // Front
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 }, .normal { 0, 0, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 }, .normal { 0, 0, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 }, .normal { 0, 0, 1 } };
      m += vtx { .pos {  0.5,  0.5,  0.5 }, .uv { 0, 1 }, .normal { 0, 0, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 }, .normal { 0, 0, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 }, .normal { 0, 0, 1 } };

      // Back
      m += vtx { .pos { -0.5, -0.5, -0.5 }, .uv { 0, 0 }, .normal { 0, 0, -1 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 0, 1 }, .normal { 0, 0, -1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 0 }, .normal { 0, 0, -1 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 }, .normal { 0, 0, -1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 0 }, .normal { 0, 0, -1 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 0, 1 }, .normal { 0, 0, -1 } };

      // Bottom
      m += vtx { .pos { -0.5, -0.5, -0.5 }, .uv { 1, 1 }, .normal { 0, -1, 0 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 0, 1 }, .normal { 0, -1, 0 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 }, .normal { 0, -1, 0 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 }, .normal { 0, -1, 0 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 }, .normal { 0, -1, 0 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 0, 1 }, .normal { 0, -1, 0 } };

      // Top
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 1, 1 }, .normal { 0, 1, 0 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 0 }, .normal { 0, 1, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 0, 1 }, .normal { 0, 1, 0 } };
      m += vtx { .pos {  0.5,  0.5,  0.5 }, .uv { 0, 0 }, .normal { 0, 1, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 0, 1 }, .normal { 0, 1, 0 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 0 }, .normal { 0, 1, 0 } };

      // Left
      m += vtx { .pos { -0.5, -0.5, -0.5 }, .uv { 0, 0 }, .normal { -1, 0, 0 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 }, .normal { -1, 0, 0 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 0, 1 }, .normal { -1, 0, 0 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 }, .normal { -1, 0, 0 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 0, 1 }, .normal { -1, 0, 0 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 }, .normal { -1, 0, 0 } };

      // Right
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 0 }, .normal { 1, 0, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 }, .normal { 1, 0, 0 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 }, .normal { 1, 0, 0 } };
      m += vtx { .pos {  0.5,  0.5,  0.5 }, .uv { 0, 1 }, .normal { 1, 0, 0 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 }, .normal { 1, 0, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 }, .normal { 1, 0, 0 } };
    }
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
