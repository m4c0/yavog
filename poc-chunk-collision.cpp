#pragma leco app
import buffers;
import chunk;
import dotz;
import models;
import silog;
import voo;

int main() {
  voo::device_and_queue dq { "poc-chunk-collision" };

  static constexpr const unsigned len = 32; // PoT padding
  chunk::gpunator cgpu {
    len,
    models::corner::t {},
    models::cube::t {},
    models::prism::t {},
  };
  chunk::stamp ch = cgpu.stamp();

  using enum chunk::model;
  constexpr const auto mm = chunk::minmax;
  for (auto x = -mm; x <= mm; x++) {
    for (auto y = -mm; y <= -2; y++) {
      for (auto z = -mm; z <= mm; z++) {
        ch.set({ x, y, z }, { .mdl = cube });
      }
    }
  }
  const dotz::vec4 r90 { 0, 0.7071, 0, 0.7071 };
  const dotz::vec4 r180 { 0, 1, 0, 0 };
  const dotz::vec4 r270 { 0, -0.7071, 0, 0.7071 };
  for (auto x = -1; x <= 1; x++) {
    for (auto y = -1; y <= 1; y++) {
      ch.set({ 3 + x, -1, 5 + y }, { .mdl = cube });
    }
    ch.set({ 5, -1, 5 + x }, {              .mdl = prism });
    ch.set({ 1, -1, 5 + x }, { .rot = r180, .mdl = prism });
    ch.set({ 3 + x, -1, 3 }, { .rot = r90,  .mdl = prism });
    ch.set({ 3 + x, -1, 7 }, { .rot = r270, .mdl = prism });
  }
  ch.set({ 5, -1, 7 }, {              .mdl = corner });
  ch.set({ 1, -1, 7 }, { .rot = r270, .mdl = corner });
  ch.set({ 5, -1, 3 }, { .rot = r90,  .mdl = corner });
  ch.set({ 1, -1, 3 }, { .rot = r180, .mdl = corner });

  ch.copy({});
  cgpu.submit();

  voo::single_cb cb {};
  // create compute pipeline
  // extract number of instances after compact
  // dispatch
  // read from host
  // take a list of candidates (might be useful for wall-hugging and enemies)

  auto col = cgpu.collision();
  voo::run(voo::cmd_buf_one_time_submit { cb.cb() }, [&] {
    col.cmd(cb.cb());
  });

  vee::device_wait_idle();

  auto m = cgpu.dcmd().map();
  silog::infof("%d %d %d", m[0].x, m[0].y, m[0].z);
}
