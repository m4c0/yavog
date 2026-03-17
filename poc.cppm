export module poc;
import casein;
import voo;

namespace poc {
  export inline auto device_and_queue(const char * name) {
    auto dn = vee::physical_device_vulkan11_features({
      .multiview = true,
    });
    return voo::device_and_queue { name, casein::native_ptr, {
      .feats {
        .independentBlend = true,
        .samplerAnisotropy = true,
      },
      .next = &dn,
    }};
  }
}
