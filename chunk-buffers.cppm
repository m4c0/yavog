export module chunk:buffers;
import buffers;
import vee;

namespace chunk {
  inline constexpr const auto max_collisions = 8;

  export struct buffers {
    ::buffers::v_buffer  vtx;
    ::buffers::ix_buffer idx;
    ::buffers::e_buffer  edg;
    ::buffers::vc_buffer vcmd;
    ::buffers::ec_buffer ecmd;
    ::buffers::dc_buffer dcmd { max_collisions };
    ::buffers::i_buffer  inst;

    template<typename... T> buffers(unsigned insts, T...) :
      vtx  { T {}... }
    , idx  { T {}... }
    , edg  { T {}... }
    , vcmd { sizeof...(T) + 1 }
    , ecmd { sizeof...(T) + 1 }
    , inst { insts }
    {}

    void cmd_draw_vtx(VkCommandBuffer cb) {
      vtx.cmd_bind(cb);
      idx.cmd_bind(cb);
      inst.cmd_bind(cb);
      vcmd.cmd_draw(cb);
    }
    void cmd_draw_edg(VkCommandBuffer cb) {
      edg.cmd_bind(cb);
      inst.cmd_bind(cb);
      ecmd.cmd_draw(cb);
    }
  };
}
