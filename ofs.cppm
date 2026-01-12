export module ofs;
import voo;

export namespace ofs {
  auto render_pass() {
    return vee::create_render_pass({
      .attachments {{
        vee::create_colour_attachment(
            VK_FORMAT_R8G8B8A8_SRGB,
            vee::image_layout_shader_read_only_optimal),
        vee::create_depth_attachment(),
      }},
      .subpasses {{
        vee::create_subpass({
          .colours {{ vee::create_attachment_ref(0, vee::image_layout_color_attachment_optimal) }},
          .depth_stencil = vee::create_attachment_ref(1, vee::image_layout_depth_stencil_attachment_optimal),
        }),
      }},
      .dependencies {{
        vee::create_colour_dependency(),
        vee::create_depth_dependency(),
      }},
    });
  }

  struct framebuffer {
    voo::bound_image colour;
    voo::bound_image depth;
    vee::render_pass rp;
    vee::framebuffer fb;

    explicit framebuffer(vee::extent ext) :
      colour { voo::bound_image::create(
          ext, VK_FORMAT_R8G8B8A8_SRGB,
          vee::image_usage_colour_attachment, vee::image_usage_sampled) }
    , depth { voo::bound_image::create_depth(ext) }
    , rp { render_pass() }
    , fb { vee::create_framebuffer({
      .render_pass = *rp,
      .attachments {{
        *colour.iv,
        *depth.iv,
      }},
      .extent = ext,
    }) }
    {}
  };
}
