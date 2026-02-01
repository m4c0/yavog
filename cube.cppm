export module cube;
import dotz;
import clay;
import ofs;
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
  struct v_buffer : public clay::buffer<ofs::vtx>, no::no {
    v_buffer() : clay::buffer<ofs::vtx> { 37 } {
      auto m = map();

      // Front
      m += { .pos = vtxes[6], .uv { 1, 0 }, .normal { 0, 0, 1 } };
      m += { .pos = vtxes[2], .uv { 0, 0 }, .normal { 0, 0, 1 } };
      m += { .pos = vtxes[4], .uv { 1, 1 }, .normal { 0, 0, 1 } };
      m += { .pos = vtxes[0], .uv { 0, 1 }, .normal { 0, 0, 1 } };

      // Back
      m += { .pos = vtxes[7], .uv { 0, 0 }, .normal { 0, 0, -1 } };
      m += { .pos = vtxes[5], .uv { 0, 1 }, .normal { 0, 0, -1 } };
      m += { .pos = vtxes[3], .uv { 1, 0 }, .normal { 0, 0, -1 } };
      m += { .pos = vtxes[1], .uv { 1, 1 }, .normal { 0, 0, -1 } };

      // Bottom
      m += { .pos = vtxes[7], .uv { 1, 1 }, .normal { 0, -1, 0 } };
      m += { .pos = vtxes[3], .uv { 0, 1 }, .normal { 0, -1, 0 } };
      m += { .pos = vtxes[6], .uv { 1, 0 }, .normal { 0, -1, 0 } };
      m += { .pos = vtxes[2], .uv { 0, 0 }, .normal { 0, -1, 0 } };

      // Top
      m += { .pos = vtxes[5], .uv { 1, 1 }, .normal { 0, 1, 0 } };
      m += { .pos = vtxes[4], .uv { 1, 0 }, .normal { 0, 1, 0 } };
      m += { .pos = vtxes[1], .uv { 0, 1 }, .normal { 0, 1, 0 } };
      m += { .pos = vtxes[0], .uv { 0, 0 }, .normal { 0, 1, 0 } };

      // Left
      m += { .pos = vtxes[7], .uv { 0, 0 }, .normal { -1, 0, 0 } };
      m += { .pos = vtxes[6], .uv { 1, 0 }, .normal { -1, 0, 0 } };
      m += { .pos = vtxes[5], .uv { 0, 1 }, .normal { -1, 0, 0 } };
      m += { .pos = vtxes[4], .uv { 1, 1 }, .normal { -1, 0, 0 } };

      // Right
      m += { .pos = vtxes[3], .uv { 1, 0 }, .normal { 1, 0, 0 } };
      m += { .pos = vtxes[1], .uv { 1, 1 }, .normal { 1, 0, 0 } };
      m += { .pos = vtxes[2], .uv { 0, 0 }, .normal { 1, 0, 0 } };
      m += { .pos = vtxes[0], .uv { 0, 1 }, .normal { 1, 0, 0 } };
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

  struct shadow_v_buffer : public clay::buffer<ofs::edge>, no::no {
    shadow_v_buffer() : clay::buffer<ofs::edge> { 36 } {
      auto m = map();

      const auto tri = [&](ofs::edge e) {
        m += {};
        e.nrm_a.w = 1; m += e;
        e.nrm_a.w = 2; m += e;
      };

      tri({ {  0,  0,  1, 0 }, { -1,  0,  0, 0 }, vtxes[6], vtxes[4] });
      tri({ {  0,  0,  1, 0 }, {  1,  0,  0, 0 }, vtxes[0], vtxes[2] });
      tri({ {  0,  0,  1, 0 }, {  0, -1,  0, 0 }, vtxes[2], vtxes[6] });
      tri({ {  0,  0,  1, 0 }, {  0,  1,  0, 0 }, vtxes[4], vtxes[0] });

      tri({ {  0,  0, -1, 0 }, { -1,  0,  0, 0 }, vtxes[5], vtxes[7] });
      tri({ {  0,  0, -1, 0 }, {  1,  0,  0, 0 }, vtxes[3], vtxes[1] });
      tri({ {  0,  0, -1, 0 }, {  0, -1,  0, 0 }, vtxes[7], vtxes[3] });
      tri({ {  0,  0, -1, 0 }, {  0,  1,  0, 0 }, vtxes[1], vtxes[5] });

      tri({ {  0, -1,  0, 0 }, { -1,  0,  0, 0 }, vtxes[7], vtxes[6] });
      tri({ {  0, -1,  0, 0 }, {  1,  0,  0, 0 }, vtxes[2], vtxes[3] });

      tri({ {  0,  1,  0, 0 }, { -1,  0,  0, 0 }, vtxes[4], vtxes[5] });
      tri({ {  0,  1,  0, 0 }, {  1,  0,  0, 0 }, vtxes[1], vtxes[0] });
    }
  };

  struct i_buffer : clay::buffer<ofs::inst>, no::no {
    i_buffer() : clay::buffer<ofs::inst> { 128 * 128 * 2} {
    }
  };

  class drawer : public ofs::drawer {
    v_buffer cube {};
    i_buffer insts {};
    shadow_v_buffer shdvtx {};
    voo::bound_buffer idx = cube::ix_buffer();

  public:
    [[nodiscard]] auto map() { return insts.map(); }

    void faces(vee::command_buffer cb, vee::pipeline_layout::type pl) override {
      vee::cmd_bind_vertex_buffers(cb, 0, *cube, 0);
      vee::cmd_bind_vertex_buffers(cb, 1, *insts, 0);
      vee::cmd_bind_index_buffer_u16(cb, *idx.buffer);
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
