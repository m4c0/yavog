export module timing;
import casein;
import dotz;
import no;
import silog;
import traits;
import voo;

using namespace traits::ints;

namespace timing {
  struct q {
    uint64_t begin;
    uint64_t postshadow;
    uint64_t prepost;
    uint64_t end;
  };

  class counters {
    dotz::vec2 m_wnd_size = casein::window_size;
    uint64_t m_frames = 0;
    uint64_t m_total = 0;
    uint64_t m_post = 0;

  public:
    void print() {
      if (m_frames == 0) return;

      float tp = vee::get_physical_device_properties().limits.timestampPeriod;
      silog::infof("Average timings per frame after %d frames (resolution: %.0fx%.0f)",
          m_frames, m_wnd_size.x, m_wnd_size.y);
      silog::infof("-- Post-FX: %7.3fms", m_post   * tp / (m_frames * 1000'000));
      silog::infof("-- All:     %7.3fms", m_total  * tp / (m_frames * 1000'000));
    }

    void add(q q) {
      m_frames++;
      m_total  += q.end - q.begin;
      m_post   += q.end - q.prepost;
    }
  } g_counter {};

  export class query : no::no {
    voo::bound_buffer m_buf = voo::bound_buffer::create_from_host(sizeof(q), VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vee::query_pool m_qp = vee::create_timestamp_query_pool(4);

    void read() {
      voo::memiter<q> m { *m_buf.memory };
      g_counter.add(m[0]);
    }

  public:
    ~query() {
      g_counter.print();
      g_counter = {};
    };

    void write_begin(vee::command_buffer cb) {
      read();

      vee::cmd_reset_query_pool(cb, *m_qp, 0, 4);
      vee::cmd_write_timestamp(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, *m_qp, 0);
    }
    void write_prepost(vee::command_buffer cb) {
      vee::cmd_write_timestamp(cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, *m_qp, 1);
    }
    void write_end(vee::command_buffer cb) {
      vee::cmd_write_timestamp(cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, *m_qp, 2);
      vee::cmd_copy_query_pool_results(cb, *m_qp, 0, 3, *m_buf.buffer);
    }
  };
}
