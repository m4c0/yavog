export module camera;
import casein;
import dotz;

using namespace casein;

namespace camera {
  export class t {
    dotz::vec3 m_pos {};
    dotz::vec2 m_rot {};
    bool m_down[4] {};

    void warp_cursor() {
      mouse_pos = window_size / 2.0;
      interrupt(IRQ_MOUSE_POS);
    }
    float axis(unsigned ia, unsigned ib) const {
      auto a = m_down[ia];
      auto b = m_down[ib];
      if (a == b) return 0;
      if (a) return -1;
      return b ? 1 : 0;
    }

  public:
    t() {
      cursor_visible = false;
      interrupt(IRQ_CURSOR);
      warp_cursor();

      handle(MOUSE_MOVE, { this, &t::warp_cursor });
      handle(MOUSE_MOVE_REL, [this] {
        auto [yaw, pitch] = mouse_rel;
        m_rot.x = dotz::clamp(m_rot.x - pitch, -90.f, 90.f);
        m_rot.y += yaw;
      });

      handle(KEY_DOWN, K_W, [this] { m_down[0] = true; });
      handle(KEY_DOWN, K_A, [this] { m_down[1] = true; });
      handle(KEY_DOWN, K_S, [this] { m_down[2] = true; });
      handle(KEY_DOWN, K_D, [this] { m_down[3] = true; });
      handle(KEY_UP, K_W, [this] { m_down[0] = false; });
      handle(KEY_UP, K_A, [this] { m_down[1] = false; });
      handle(KEY_UP, K_S, [this] { m_down[2] = false; });
      handle(KEY_UP, K_D, [this] { m_down[3] = false; });
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

    [[nodiscard]] constexpr auto position()  const { return m_pos; }
    [[nodiscard]] constexpr auto rotation()  const { return m_rot; }

    [[nodiscard]] constexpr auto direction() const {
      return dotz::vec3 { axis(1, 3), 0, axis(0, 2) };
    }

    [[nodiscard]] constexpr auto lerp(float f) { return m_pos + direction() * f; }
    constexpr void position(dotz::vec3 p) { m_pos = p; }
  };
}
