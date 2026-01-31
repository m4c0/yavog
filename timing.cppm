export module timing;
import casein;
import dotz;
import no;
import silog;
import traits;
import voo;

using namespace traits::ints;

namespace timing {
  export enum ppl {
    ppl_render,
    ppl_post,
    ppl_max,
  };

  class counters {
    dotz::vec2 m_wnd_size = casein::window_size;
    uint64_t m_frames = 0;
    uint64_t m_acc_total = 0;
    uint64_t m_acc[ppl_max] {};

    void print(const char * str, uint64_t p, float tp) {
      silog::infof("-- %12s: %7.3fms", str, p * tp / (m_frames * 1000'000));
    }

  public:
    void print() {
      if (m_frames == 0) return;

      float tp = vee::get_physical_device_properties().limits.timestampPeriod;
      silog::infof("Average timings per frame after %d frames (resolution: %.0fx%.0f)",
          m_frames, m_wnd_size.x, m_wnd_size.y);

      print("Render",       m_acc[ppl_render],  tp);
      print("Post-FX",      m_acc[ppl_post],    tp);
      print("All",          m_acc_total,        tp);
    }

    void add(uint64_t * qq) {
      m_frames++;
      for (auto i = 0; i < ppl_max; i++) {
        m_acc[i] += qq[i + 1] - qq[i];
      }
      m_acc_total += qq[ppl_max] - qq[0];
    }
  } g_counter {};

  export class query : no::no {
    static constexpr const auto num_qry = ppl_max + 1;
    static constexpr const auto buf_size = sizeof(uint64_t) * num_qry;

    voo::bound_buffer m_buf = voo::bound_buffer::create_from_host(buf_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vee::query_pool m_qp = vee::create_timestamp_query_pool(num_qry);

    void read() {
      voo::memiter<uint64_t> m { *m_buf.memory };
      g_counter.add(&m[0]);
    }

  public:
    ~query() {
      g_counter.print();
      g_counter = {};
    };

    void write(vee::command_buffer cb, auto && fn) {
      read();

      vee::cmd_reset_query_pool(cb, *m_qp, 0, num_qry);
      fn();
      vee::cmd_write_timestamp(cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, *m_qp, ppl_max);
      vee::cmd_copy_query_pool_results(cb, *m_qp, 0, num_qry, *m_buf.buffer);
    }
    void write(ppl pp, vee::command_buffer cb) {
      vee::cmd_write_timestamp(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, *m_qp, pp);
    }
  };
}
