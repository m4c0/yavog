#pragma leco app
import buffers;
import chunk;
import silog;
import voo;

using inst = buffers::tmp_inst;

int main() {
  voo::device_and_queue dq { "poc-chunk-parts" };

  constexpr const auto len = 32;
  constexpr const auto buf_sz = sizeof(inst) * len * len * len;
  voo::bound_buffer b0 = voo::bound_buffer::create_from_host(buf_sz, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  voo::bound_buffer b1 = voo::bound_buffer::create_from_host(buf_sz, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  voo::bound_buffer b2 = voo::bound_buffer::create_from_host(buf_sz, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

  chunk::compact cc { *b0.buffer, *b1.buffer, *b2.buffer, len };

  voo::single_cb cb {};

  chunk::t ch {};
  for (auto z = -1; z <= 1; z++) {
    for (auto y = -1; y <= 1; y++) {
      for (auto x = -1; x <= 1; x++) {
        ch.set({ x, y, z }, { .mdl = chunk::model::cube });
      }
    }
  }

  {
    voo::memiter<inst> m { *b0.memory };
    ch.copy(m, {}, len);
  }

  {
    voo::cmd_buf_one_time_submit ots { cb.cb() };
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
        "%3d - %.0f - %4.1f,%4.1f,%4.1f - %4.1f,%4.1f,%4.1f",
        i, m[i].mdl, px,py,pz, sx,sy,sz);
  }
}
