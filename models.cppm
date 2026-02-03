export module models;
import dotz;

namespace models {
  export struct vtx {
    unsigned id;
    dotz::vec2 uv;
    dotz::vec3 normal;
  };
  export struct tri {
    unsigned short a, b, c;
  };
}
