export module prism;
import clay;
import dotz;
import no;
import ofs;

constexpr dotz::vec4 vtxes[] {
  {  0.5, -0.5,  0.5, 1.0 }, // 0
  {  0.5, -0.5, -0.5, 1.0 }, // 1
  { -0.5,  0.5,  0.5, 1.0 }, // 2
  { -0.5,  0.5, -0.5, 1.0 }, // 3
  { -0.5, -0.5,  0.5, 1.0 }, // 4
  { -0.5, -0.5, -0.5, 1.0 }, // 5
};

export namespace prism {
  struct v_buffer : ofs::v_buffer {
    v_buffer() : ofs::v_buffer { 36 } {
      auto m = map();

      auto s = dotz::sqrt(2.0f) / 2.0f;

      m += { .pos = vtxes[5], .uv { 1, 0 }, .normal {  0,  0, -1 } };
      m += { .pos = vtxes[3], .uv { 1, 1 }, .normal {  0,  0, -1 } };
      m += { .pos = vtxes[1], .uv { 0, 0 }, .normal {  0,  0, -1 } };

      m += { .pos = vtxes[0], .uv { 1, 0 }, .normal {  0,  0,  1 } };
      m += { .pos = vtxes[2], .uv { 0, 1 }, .normal {  0,  0,  1 } };
      m += { .pos = vtxes[4], .uv { 0, 0 }, .normal {  0,  0,  1 } };

      m += { .pos = vtxes[2], .uv { 0, 1 }, .normal { -1,  0,  0 } };
      m += { .pos = vtxes[3], .uv { 1, 1 }, .normal { -1,  0,  0 } };
      m += { .pos = vtxes[4], .uv { 0, 0 }, .normal { -1,  0,  0 } };
      m += { .pos = vtxes[5], .uv { 1, 0 }, .normal { -1,  0,  0 } };

      m += { .pos = vtxes[0], .uv { 1, 0 }, .normal {  0, -1,  0 } };
      m += { .pos = vtxes[1], .uv { 1, 1 }, .normal {  0, -1,  0 } };
      m += { .pos = vtxes[4], .uv { 0, 0 }, .normal {  0, -1,  0 } };
      m += { .pos = vtxes[5], .uv { 0, 1 }, .normal {  0, -1,  0 } };

      m += { .pos = vtxes[0], .uv { 1, 0 }, .normal {  s,  s,  0 } };
      m += { .pos = vtxes[1], .uv { 1, 1 }, .normal {  s,  s,  0 } };
      m += { .pos = vtxes[2], .uv { 0, 0 }, .normal {  s,  s,  0 } };
      m += { .pos = vtxes[3], .uv { 0, 1 }, .normal {  s,  s,  0 } };
    }
  };

  struct ix_buffer : ofs::ix_buffer {
    ix_buffer() : ofs::ix_buffer { 36 } {
      auto m = map();
      m += {{ 0, 1, 2 }};
      m += {{ 3, 4, 5 }};

      m += {{  6,  7,  8 }}; m += {{  9,  8,  7 }};
      m += {{ 12, 11, 10 }}; m += {{ 11, 12, 13 }};
      m += {{ 14, 15, 16 }}; m += {{ 17, 16, 15 }};
    }
  };
}
