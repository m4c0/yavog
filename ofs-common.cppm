export module ofs:common;
import jute;
import msaa;
import sv;
import traits;
import voo;

using namespace wagen;

inline auto stencil(VkStencilOp stencil_op, VkCompareOp compare_op) {
  return VkStencilOpState {
    .failOp = VK_STENCIL_OP_KEEP,
    .passOp = stencil_op,
    .depthFailOp = VK_STENCIL_OP_KEEP,
    .compareOp = compare_op,
    .compareMask = ~0U,
    .writeMask = ~0U,
  };
}
inline auto create_graphics_pipeline(sv shader, vee::gr_pipeline_params p) {
  auto vert = voo::vert_shader(jute::fmt<"%s.vert.spv">(shader));
  auto frag = voo::frag_shader(jute::fmt<"%s.frag.spv">(shader));
  p.shaders = { *vert, *frag };
  return msaa::create_graphics_pipeline(traits::move(p));
}
inline auto create_colour_only_pipeline(sv shader, vee::gr_pipeline_params p) {
  p.blends = {
    vee::colour_blend_classic(),
    VkPipelineColorBlendAttachmentState {},
    VkPipelineColorBlendAttachmentState {},
  };
  return create_graphics_pipeline(shader, p);
}


