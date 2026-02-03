export module cube;
import dotz;
import clay;
import models;
import no;
import ofs;
import traits;
import voo;

constexpr const dotz::vec4 vtx_pos[] {
  {  0.5,  0.5,  0.5, 1.0 }, // 0
  {  0.5,  0.5, -0.5, 1.0 }, // 1
  {  0.5, -0.5,  0.5, 1.0 }, // 2
  {  0.5, -0.5, -0.5, 1.0 }, // 3

  { -0.5,  0.5,  0.5, 1.0 }, // 4
  { -0.5,  0.5, -0.5, 1.0 }, // 5
  { -0.5, -0.5,  0.5, 1.0 }, // 6
  { -0.5, -0.5, -0.5, 1.0 }, // 7
};
constexpr const models::vtx vtx[] {
  { .id = 6, .uv { 1, 0 }, .normal { 0, 0, 1 } },
  { .id = 2, .uv { 0, 0 }, .normal { 0, 0, 1 } },
  { .id = 4, .uv { 1, 1 }, .normal { 0, 0, 1 } },
  { .id = 0, .uv { 0, 1 }, .normal { 0, 0, 1 } },

  // Back
  { .id = 7, .uv { 0, 0 }, .normal { 0, 0, -1 } },
  { .id = 5, .uv { 0, 1 }, .normal { 0, 0, -1 } },
  { .id = 3, .uv { 1, 0 }, .normal { 0, 0, -1 } },
  { .id = 1, .uv { 1, 1 }, .normal { 0, 0, -1 } },

  // Bottom
  { .id = 7, .uv { 1, 1 }, .normal { 0, -1, 0 } },
  { .id = 3, .uv { 0, 1 }, .normal { 0, -1, 0 } },
  { .id = 6, .uv { 1, 0 }, .normal { 0, -1, 0 } },
  { .id = 2, .uv { 0, 0 }, .normal { 0, -1, 0 } },

  // Top
  { .id = 5, .uv { 0, 1 }, .normal { 0, 1, 0 } },
  { .id = 4, .uv { 0, 0 }, .normal { 0, 1, 0 } },
  { .id = 1, .uv { 1, 1 }, .normal { 0, 1, 0 } },
  { .id = 0, .uv { 1, 0 }, .normal { 0, 1, 0 } },

  // Left
  { .id = 7, .uv { 0, 0 }, .normal { -1, 0, 0 } },
  { .id = 6, .uv { 1, 0 }, .normal { -1, 0, 0 } },
  { .id = 5, .uv { 0, 1 }, .normal { -1, 0, 0 } },
  { .id = 4, .uv { 1, 1 }, .normal { -1, 0, 0 } },

  // Right
  { .id = 3, .uv { 1, 0 }, .normal { 1, 0, 0 } },
  { .id = 1, .uv { 1, 1 }, .normal { 1, 0, 0 } },
  { .id = 2, .uv { 0, 0 }, .normal { 1, 0, 0 } },
  { .id = 0, .uv { 0, 1 }, .normal { 1, 0, 0 } },
};
constexpr const models::tri tri[] {
  {  0,  1,  2 }, {  3,  2,  1 }, // Front
  {  4,  5,  6 }, {  7,  6,  5 }, // Back
  {  8,  9, 10 }, { 11, 10,  9 }, // Bottom
  { 12, 13, 14 }, { 15, 14, 13 }, // Top
  { 16, 17, 18 }, { 19, 18, 17 }, // Left
  { 20, 21, 22 }, { 23, 22, 21 }, // Right
};

export namespace cube {
  struct v_buffer : public ofs::v_buffer {
    v_buffer() : ofs::v_buffer { vtx, vtx_pos } {}
  };

  struct ix_buffer : ofs::ix_buffer {
    ix_buffer() : ofs::ix_buffer { tri } {}
  };

  struct shadow_v_buffer : public ofs::e_buffer {
    shadow_v_buffer() : e_buffer { 36 } {
      auto m = map();

      tri(m, { {  0,  0,  1, 0 }, { -1,  0,  0, 0 }, vtx_pos[6], vtx_pos[4] });
      tri(m, { {  0,  0,  1, 0 }, {  1,  0,  0, 0 }, vtx_pos[0], vtx_pos[2] });
      tri(m, { {  0,  0,  1, 0 }, {  0, -1,  0, 0 }, vtx_pos[2], vtx_pos[6] });
      tri(m, { {  0,  0,  1, 0 }, {  0,  1,  0, 0 }, vtx_pos[4], vtx_pos[0] });

      tri(m, { {  0,  0, -1, 0 }, { -1,  0,  0, 0 }, vtx_pos[5], vtx_pos[7] });
      tri(m, { {  0,  0, -1, 0 }, {  1,  0,  0, 0 }, vtx_pos[3], vtx_pos[1] });
      tri(m, { {  0,  0, -1, 0 }, {  0, -1,  0, 0 }, vtx_pos[7], vtx_pos[3] });
      tri(m, { {  0,  0, -1, 0 }, {  0,  1,  0, 0 }, vtx_pos[1], vtx_pos[5] });

      tri(m, { {  0, -1,  0, 0 }, { -1,  0,  0, 0 }, vtx_pos[7], vtx_pos[6] });
      tri(m, { {  0, -1,  0, 0 }, {  1,  0,  0, 0 }, vtx_pos[2], vtx_pos[3] });

      tri(m, { {  0,  1,  0, 0 }, { -1,  0,  0, 0 }, vtx_pos[4], vtx_pos[5] });
      tri(m, { {  0,  1,  0, 0 }, {  1,  0,  0, 0 }, vtx_pos[1], vtx_pos[0] });
    }
  };

  struct i_buffer : clay::buffer<ofs::inst>, no::no {
    i_buffer() : clay::buffer<ofs::inst> { 128 * 128 * 2} {
    }
  };

  class drawer : public ofs::drawer {
    v_buffer vtxs {};
    i_buffer insts {};
    shadow_v_buffer shdvtx {};
    ix_buffer idx {};

  public:
    [[nodiscard]] auto map() { return insts.map(); }

    void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
      vee::cmd_bind_vertex_buffers(cb, 0, *vtxs, 0);
      vee::cmd_bind_vertex_buffers(cb, 1, *insts, 0);
      vee::cmd_bind_index_buffer_u16(cb, *idx);
      vee::cmd_draw_indexed(cb, {
        .xcount = 36,
        .icount = insts.count(),
      });
    }
    void edges(vee::command_buffer cb) override {
      vee::cmd_bind_vertex_buffers(cb, 0, *shdvtx, 0);
      vee::cmd_draw(cb, {
        .vcount = 36,
        .icount = insts.count(),
      });
    }
  };
}
