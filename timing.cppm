export module timing;
import silog;
import traits;
import voo;

using namespace traits::ints;

namespace timing {
  struct q {
    uint64_t begin;
    uint64_t end;
  };

  export class query {
    voo::bound_buffer m_buf = voo::bound_buffer::create_from_host(sizeof(q), vee::buffer_usage::transfer_dst_buffer);
    vee::query_pool m_qp = vee::create_timestamp_query_pool(sizeof(q));
    float m_tp = vee::get_physical_device_properties().limits.timestampPeriod;

    void read() {
      voo::memiter<q> m { *m_buf.memory };
      auto q = m[0];

      auto total = (q.end - q.begin) * m_tp;
      silog::infof("%fms", total / 1000'000);
    }

  public:
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
