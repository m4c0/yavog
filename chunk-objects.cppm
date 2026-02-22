export module chunk:objects;

namespace chunk {
  export enum class model : unsigned {
    none = 0,
    corner,
    cube,
    prism,
    max,
  };
  export constexpr const auto model_count = static_cast<unsigned>(model::max);
}
