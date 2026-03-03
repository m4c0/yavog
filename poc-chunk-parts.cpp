#pragma leco app
import buffers;
import chunk;
import silog;
import voo;

using inst = buffers::inst;

int main() {
  voo::device_and_queue dq { "poc-chunk-parts" };

  constexpr const auto len = 64;
  constexpr const auto buf_sz = sizeof(inst) * len * len * len;
  voo::bound_buffer b0 = voo::bound_buffer::create_from_host(buf_sz, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  voo::bound_buffer b1 = voo::bound_buffer::create_from_host(buf_sz, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

  chunk::stamp ch { *b0.buffer, len };
  chunk::compact cc { *b0.buffer, *b1.buffer, len };

  voo::single_cb cb {};

  for (auto z = -1; z <= 1; z++) {
    for (auto y = -1; y <= 1; y++) {
      for (auto x = -1; x <= 1; x++) {
        ch.set({ x, y, z }, { .mdl = chunk::model::cube, .txt = 67 });
      }
    }
  }
    for (auto i = 0; i < 31 * 31 * 31; i++) {
      auto m = ch.data();
      if (m[i].mdl == chunk::model::none) continue;

      silog::infof(
          "ch: %3d - mdl:%d txt:%d",
          i, m[i].mdl, m[i].txt);
    }
  {
    voo::cmd_buf_one_time_submit ots { cb.cb() };
    ch.cmd(cb.cb(), {});
  }
  voo::queue::universal()->queue_submit({ .command_buffer = cb.cb() });
  vee::device_wait_idle();
  {
    voo::memiter<inst> m { *b0.memory };
    for (auto i = 0; i < len * len * len; i++) {
      if (m[i].mdl == 0) continue;

      auto [px,py,pz] = m[i].pos;
      auto [sx,sy,sz] = m[i].size;

      silog::infof(
          "b0: %3d - mdl:%.0f txt:%.0f - pos:%4.1f,%4.1f,%4.1f - sz:%4.1f,%4.1f,%4.1f",
          i, m[i].mdl, m[i].txtid, px,py,pz, sx,sy,sz);
    }
  }

  {
    voo::cmd_buf_one_time_submit ots { cb.cb() };
    ch.cmd(cb.cb(), {});
    cc.cmd(cb.cb());
  }
  voo::queue::universal()->queue_submit({ .command_buffer = cb.cb() });

  vee::device_wait_idle();

  voo::memiter<inst> m { *b1.memory };
  for (auto i = 0; i < len * len * len; i++) {
    if (m[i].mdl == 0) continue;

    auto [px,py,pz] = m[i].pos;
    auto [sx,sy,sz] = m[i].size;

    silog::infof(
        "b1: %3d - mdl:%.0f txt:%.0f - pos:%4.1f,%4.1f,%4.1f - sz:%4.1f,%4.1f,%4.1f",
        i, m[i].mdl, m[i].txtid, px,py,pz, sx,sy,sz);
  }
}
