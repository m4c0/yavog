#pragma leco test
import print;
import voo;

inline bool chkfmt(VkFormat fmt, VkFormatFeatureFlags flags) {
  auto p = vee::get_physical_device_properties(fmt);
  return (p.optimalTilingFeatures & flags) == flags;
}

int main() {
  voo::device_and_queue dq { "poc-diffmt" };

  auto bits = VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;

  // Rarely supported format - this confirms we don't explode or something
  if (chkfmt(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG, bits)) return 1;
  if (!chkfmt(VK_FORMAT_A1R5G5B5_UNORM_PACK16, bits)) return 2;
}
