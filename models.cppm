export module models;
export import :corner;
export import :cube;
export import :prism;
import embedded;

namespace models {
  export struct drawer : embedded::drawer {
    drawer(unsigned max_ins) : embedded::drawer {
      max_ins,
      models::corner::t {},
      models::cube::t {},
      models::prism::t {},
    } {}

    [[nodiscard]] auto builder() { return embedded::builder { *this }; }
  };
}
