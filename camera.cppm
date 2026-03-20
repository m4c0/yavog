export module camera;
import casein;
import dotz;

using namespace casein;

namespace camera {
  export class t {
    dotz::vec3 m_pos {};
    dotz::vec3 m_dir {};
    dotz::vec2 m_rot {};

    void warp_cursor() {
      mouse_pos = window_size / 2.0;
      interrupt(IRQ_MOUSE_POS);
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
    }

    [[nodiscard]] constexpr auto direction() const { return m_dir; }
    [[nodiscard]] constexpr auto position()  const { return m_pos; }
    [[nodiscard]] constexpr auto rotation()  const { return m_rot; }

    [[nodiscard]] constexpr auto lerp(float f) { return m_pos + m_dir * f; }
    constexpr void position(dotz::vec3 p) { m_pos = p; }
  };
}
