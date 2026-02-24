export module indirectdebug;
import jute;
import silog;
import traits;
import voo;

using namespace jute::literals;
using namespace traits::ints;
using namespace wagen;

namespace indirectdebug {
  export struct indexed_indirect_params {
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
  }
}
