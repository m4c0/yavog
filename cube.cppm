export module cube;
import dotz;
import clay;

export namespace cube {
  struct vtx {
    dotz::vec3 pos;
    float _pad;
    dotz::vec2 uv;
    dotz::vec3 normal;
  };

  struct buffer : public clay::buffer<vtx> {
    buffer() : clay::buffer<vtx> { 36 } {
      auto m = map();

      // Front
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5,  0.5,  0.5 }, .uv { 0, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 } };

      // Back
      m += vtx { .pos { -0.5, -0.5, -0.5 }, .uv { 0, 0 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 0, 1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 0 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 0, 1 } };

      // Bottom
      m += vtx { .pos { -0.5, -0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 0, 1 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 0, 1 } };

      // Top
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 0, 1 } };
      m += vtx { .pos {  0.5,  0.5,  0.5 }, .uv { 0, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 0, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 0 } };

      // Left
      m += vtx { .pos { -0.5, -0.5, -0.5 }, .uv { 0, 0 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 0, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 0, 1 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 0 } };

      // Right
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 } };
      m += vtx { .pos {  0.5,  0.5,  0.5 }, .uv { 0, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 0, 0 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 } };
    }
  };
}
