export module indirectdebug;
import buffers;
import dotz;
import jute;
import silog;
import traits;
import voo;

using namespace jute::literals;
using namespace traits::ints;
using namespace wagen;

namespace indirectdebug {
  export void dump_insts(unsigned count, VkDeviceMemory mem) {
    vee::device_wait_idle();

    voo::memiter<buffers::inst> m { mem };
    for (auto i = 0; i < count; i++) {
      auto [px,py,pz] = m[i].pos;
      auto [sx,sy,sz] = m[i].size;
      if (m[i].mdl == 0) continue;
      silog::infof("%d -- size:%f,%f,%f -- pos:%f,%f,%f -- mdl:%.0f - txt:%.0f", i, sx,sy,sz, px,py,pz, m[i].mdl, m[i].txtid);
    }
  }

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
  export void dump(unsigned count, indexed_indirect_params p) {
    for (auto i = 0U; i < count; i++) {
      p.first = i;
      indirectdebug::dump(p);
    }
  }

  export struct indirect_params {
    VkDeviceMemory vertices;
    VkDeviceMemory indirect;
    unsigned first;
  };
  export void dump(indirect_params p) {
    vee::device_wait_idle();

    voo::memiter<VkDrawIndirectCommand> mcmd { p.indirect };
    auto mcmd_i = mcmd[p.first];

    silog::infof("Indirect instances:%d+%d vertices:%d+%d",
        mcmd_i.firstInstance, mcmd_i.instanceCount,
        mcmd_i.firstVertex, mcmd_i.vertexCount);

    struct vtx {
      dotz::vec4 pos;
      dotz::vec2 uv;
      dotz::vec3 normal;
    };
    voo::memiter<vtx> mvtx { p.vertices };
    for (auto i = 0U; i < dotz::min(12, mcmd_i.vertexCount); i++) {
      auto v = mvtx[i + mcmd_i.firstVertex];
      silog::infof("- pos:%f,%f,%f,%f -- uv:%f,%f -- normal:%f,%f,%f",
          v.pos.x, v.pos.y, v.pos.z, v.pos.w,
          v.uv.x, v.uv.y,
          v.normal.x, v.normal.y, v.normal.z);
    }
  }
  export void dump(unsigned count, indirect_params p) {
    for (auto i = 0U; i < count; i++) {
      p.first = i;
      indirectdebug::dump(p);
    }
  }

  export void dump(const buffers::all & bufs, unsigned icount, unsigned vccount) {
    vee::device_wait_idle();

    silog::info("--------- insts");
    indirectdebug::dump_insts(icount, bufs.inst.memory());
    silog::info("--------- vcmd");
    indirectdebug::dump(vccount, indirectdebug::indexed_indirect_params {
      .vertices = bufs.vtx.memory(),
      .indices = bufs.idx.memory(),
      .indirect = bufs.vcmd.memory(),
    });
    silog::info("--------- ecmd");
    indirectdebug::dump(vccount, indirectdebug::indirect_params {
      .vertices = bufs.edg.memory(),
      .indirect = bufs.ecmd.memory(),
    });
  }
}
