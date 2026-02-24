export module indirectdebug;
import silog;
import voo;

using namespace wagen;

namespace indirectdebug {
  export struct indexed_indirect_params {
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
  }
}
