export module camera;
import casein;
import dotz;

using namespace casein;

namespace camera {
  export class t {
    dotz::vec2 m_cam;

  public:
    t() {
      cursor_visible = false;
      interrupt(IRQ_CURSOR);

      handle(MOUSE_MOVE, [] {
        mouse_pos = window_size / 2.0;
        interrupt(IRQ_MOUSE_POS);
      });
      handle(MOUSE_MOVE_REL, [this] {
        auto [yaw, pitch] = mouse_rel;
        m_cam.x = dotz::clamp(m_cam.x - pitch, -90.f, 90.f);
        m_cam.y += yaw;
      });
    }

    [[nodiscard]] constexpr auto operator*() const { return m_cam; }
  };
}
