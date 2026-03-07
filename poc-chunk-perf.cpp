#pragma leco app
import buffers;
import chunk;
import dotz;
import silog;
import sitime;
import voo;

using inst = buffers::inst;

int main() {
  voo::device_and_queue dq { "poc-chunk-parts" };
  silog::infof(
      "Max compute shared memory size: %d",
      vee::get_physical_device_properties().limits.maxComputeSharedMemorySize);

  constexpr const dotz::ivec3 len { 256, 32, 256 };
  constexpr const auto buf_n = len.x * len.y * len.z;
  constexpr const auto buf_sz = sizeof(inst) * buf_n;
  voo::bound_buffer b0 = voo::bound_buffer::create_from_device_local(buf_sz, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  voo::bound_buffer b1 = voo::bound_buffer::create_from_device_local(buf_sz, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

  chunk::stamp ch { *b0.buffer, len };
  chunk::bitonic cc { *b0.buffer, buf_n };

  voo::single_cb cb {};

  for (auto z = -1; z <= 1; z++) {
    for (auto y = -1; y <= 1; y++) {
      for (auto x = -1; x <= 1; x++) {
        ch.set({ x, y, z }, { .mdl = chunk::model::cube, .txt = 67 });
      }
    }
  }
  ch.copy({});
  vee::device_wait_idle();

  sitime::stopwatch w {};
  {
    voo::cmd_buf_one_time_submit ots { cb.cb() };
    cc.cmd(cb.cb());
  }
  voo::queue::universal()->queue_submit({ .command_buffer = cb.cb() });
  vee::device_wait_idle();

  silog::infof("-=-=-=-=-=-=-=-=-=- %d", w.millis());
}
