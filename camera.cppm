export module camera;
import casein;
import dotz;

using namespace casein;

namespace camera {
  export class t {
    enum axis_keys {
      ek_left = 0,
      ek_right,
      ek_fwd,
      ek_bwd,
    };

    dotz::vec3 m_pos {};
    float m_yaw {};
    float m_pitch {};
    bool m_down[4] {};

    void warp_cursor() {
      mouse_pos = window_size / 2.0;
      interrupt(IRQ_MOUSE_POS);
    }
    int axis(unsigned n, unsigned p) const {
      if (m_down[n] && !m_down[p]) return -1;
      if (!m_down[n] && m_down[p]) return 1;
      return 0;
    }

  public:
    t() {
      cursor_visible = false;
      interrupt(IRQ_CURSOR);
      warp_cursor();

      handle(MOUSE_MOVE, { this, &t::warp_cursor });
      handle(MOUSE_MOVE_REL, [this] {
        auto [yaw, pitch] = mouse_rel;
        m_pitch = dotz::clamp(m_pitch - pitch, -90.f, 90.f);
        m_yaw += yaw;
      });

      handle(KEY_DOWN, K_W, [this] { m_down[ek_fwd  ] = true; });
      handle(KEY_DOWN, K_A, [this] { m_down[ek_left ] = true; });
      handle(KEY_DOWN, K_S, [this] { m_down[ek_bwd  ] = true; });
      handle(KEY_DOWN, K_D, [this] { m_down[ek_right] = true; });
      handle(KEY_UP, K_W, [this] { m_down[ek_fwd  ] = false; });
      handle(KEY_UP, K_A, [this] { m_down[ek_left ] = false; });
      handle(KEY_UP, K_S, [this] { m_down[ek_bwd  ] = false; });
      handle(KEY_UP, K_D, [this] { m_down[ek_right] = false; });
    }
    ~t() {
      cursor_visible = true;
      interrupt(IRQ_CURSOR);
      reset(MOUSE_MOVE);
      reset(MOUSE_MOVE_REL);
      handle(KEY_DOWN, K_W, [] {});
      handle(KEY_DOWN, K_A, [] {});
      handle(KEY_DOWN, K_S, [] {});
      handle(KEY_DOWN, K_D, [] {});
      handle(KEY_UP, K_W, [] {});
      handle(KEY_UP, K_A, [] {});
      handle(KEY_UP, K_S, [] {});
      handle(KEY_UP, K_D, [] {});
    }

    [[nodiscard]] constexpr auto position() const { return m_pos; }
    [[nodiscard]] constexpr auto pitch() const { return m_pitch; }
    [[nodiscard]] constexpr auto yaw() const { return m_yaw; }

    [[nodiscard]] constexpr dotz::vec3 direction() const {
      float walk = axis(ek_bwd, ek_fwd);
      float strf = axis(ek_right, ek_left);
      dotz::vec2 i { strf, walk };
      if (dotz::sq_length(i) == 0) return {};
      auto d = dotz::normalise(i);

      float a = dotz::radians(m_yaw);
      float c = dotz::cos(a);
      float s = dotz::sin(a);
      return dotz::vec3 {
        d.x * c - d.y * s,
        0,
        d.x * s + d.y * c,
      };
    }

    [[nodiscard]] constexpr auto lerp(float f) { return m_pos + direction() * f; }
    constexpr void position(dotz::vec3 p) { m_pos = p; }
  };
}
