export module timing;
import no;
import silog;
import traits;
import voo;

using namespace traits::ints;

namespace timing {
  struct q {
    uint64_t begin;
    uint64_t end;
  };

  static uint64_t g_frames = 0;
  static uint64_t g_total = 0;

  export class query : no::no {
    voo::bound_buffer m_buf = voo::bound_buffer::create_from_host(sizeof(q), vee::buffer_usage::transfer_dst_buffer);
    vee::query_pool m_qp = vee::create_timestamp_query_pool(sizeof(q));
    float m_tp = vee::get_physical_device_properties().limits.timestampPeriod;

    void read() {
      voo::memiter<q> m { *m_buf.memory };
      auto q = m[0];

      g_frames++;
      g_total += q.end - q.begin;
    }

  public:
    ~query() {
      if (g_frames == 0) return;

      silog::info("Average timings per frame");
      silog::infof("-- All: %fms", g_total * m_tp / (g_frames * 1000'000));

      g_frames = 0;
    };

    void write_begin(vee::command_buffer cb) {
      read();

      vee::cmd_reset_query_pool(cb, *m_qp, 0, sizeof(q));
      vee::cmd_write_timestamp(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, *m_qp, 0);
    }
    void write_end(vee::command_buffer cb) {
      vee::cmd_write_timestamp(cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, *m_qp, 1);
      vee::cmd_copy_query_pool_results(cb, *m_qp, 0, 2, *m_buf.buffer);
    }
  };
}
