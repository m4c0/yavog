export module models:common;
import dotz;

namespace models {
  struct vtx {
    unsigned id;
    dotz::vec2 uv;
    dotz::vec3 normal;
  };
  struct tri {
    unsigned short a, b, c;
  };
  struct edge {
    unsigned short a, b;
  };
}
