export module indirectdebug;
import dotz;
import jute;
import silog;
import traits;
import voo;

using namespace jute::literals;
using namespace traits::ints;
using namespace wagen;

namespace indirectdebug {
  export struct indexed_indirect_params {
    VkDeviceMemory vertices;
    VkDeviceMemory indices;
    VkDeviceMemory indirect;
    unsigned first;
  };
  export void dump(const indexed_indirect_params & p) {
    vee::device_wait_idle();

    voo::memiter<VkDrawIndexedIndirectCommand> mcmd { p.indirect };
    auto mcmd_i = mcmd[p.first];

    silog::infof("Index. Indirect instances:%d+%d indices:%d+%d vofs:%d",
        mcmd_i.firstInstance, mcmd_i.instanceCount,
        mcmd_i.firstIndex, mcmd_i.indexCount,
        mcmd_i.vertexOffset);

    jute::heap str {};
    voo::memiter<uint16_t> midx { p.indices };
    for (auto i = 0U; i < mcmd_i.indexCount; i++) {
      auto idx = midx[i + mcmd_i.firstIndex];
      auto sep = i % 3 == 2 ? " "_s : ","_s;
      str = (str + jute::to_s(idx) + sep).heap();
    }
    silog::info(jute::fmt<"Indices: %s">(str));

    struct vtx {
      dotz::vec4 pos;
      dotz::vec2 uv;
      dotz::vec3 normal;
    };
    voo::memiter<vtx> mvtx { p.vertices };
    for (auto i = 0U; i < dotz::min(12, mcmd_i.indexCount); i++) {
      auto idx = midx[i + mcmd_i.firstIndex] + mcmd_i.vertexOffset;
      auto v = mvtx[idx];
      silog::infof("- %d -- pos:%f,%f,%f,%f -- uv:%f,%f -- normal:%f,%f,%f",
          idx,
          v.pos.x, v.pos.y, v.pos.z, v.pos.w,
          v.uv.x, v.uv.y,
          v.normal.x, v.normal.y, v.normal.z);
    }
  }
}
