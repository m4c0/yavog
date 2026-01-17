#pragma leco app
#pragma leco add_resource_dir assets
#pragma leco add_shader "poc-mcish.frag"
#pragma leco add_shader "poc-mcish.vert"

import clay;
import cube;
import dotz;
import hai;
import ofs;
import post;
import silog;
import sitime;
import sv;
import texmap;
import timing;
import traits;
import vinyl;
import voo;
import wagen;

using namespace wagen;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

static constexpr const sv t040 = "Tiles040_1K-JPG_Color.jpg";
static constexpr const sv t101 = "Tiles101_1K-JPG_Color.jpg";
static constexpr const sv t131 = "Tiles131_1K-JPG_Color.jpg";

struct upc {
  float aspect;
  float fov = 90;
};

inline VkSampleCountFlagBits max_sampling() {
  auto lim = vee::get_physical_device_properties().limits;
  auto max = lim.framebufferColorSampleCounts & lim.framebufferDepthSampleCounts;
  // if (max & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
  // if (max & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
  // if (max & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
  // if (max & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
  if (max & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
  if (max & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
  return VK_SAMPLE_COUNT_1_BIT;
}


struct app_stuff : vinyl::base_app_stuff {
  cube::v_buffer cube {};
  cube::i_buffer insts {};

  texmap::cache tmap {};
  hai::array<unsigned> txt_ids { 3 };

  post::pipeline post { dq };

  vee::pipeline_layout ofs_pl = vee::create_pipeline_layout(
      tmap.dsl(),
      vee::vertex_push_constant_range<upc>());
  vee::gr_pipeline ofs_ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *ofs_pl,
    .render_pass = *ofs::render_pass(max_sampling()),
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .multisampling = max_sampling(),
    .depth = vee::depth::op_less(),
    .blends {
      vee::colour_blend_classic(),
      vee::colour_blend_none(),
      vee::colour_blend_none(),
    },
    .shaders {
      *clay::vert_shader("poc-mcish", [] {}),
      *clay::frag_shader("poc-mcish", [] {}),
    },
    .bindings {
      cube::v_buffer::vertex_input_bind(),
      cube::i_buffer::vertex_input_bind_per_instance(),
    },
    .attributes { 
      vee::vertex_attribute_vec3(0, traits::offset_of(&cube::vtx::pos)),
      vee::vertex_attribute_vec2(0, traits::offset_of(&cube::vtx::uv)),
      vee::vertex_attribute_vec3(0, traits::offset_of(&cube::vtx::normal)),
      vee::vertex_attribute_vec4(1, traits::offset_of(&cube::inst::pos)),
    },
  });

  app_stuff() : base_app_stuff { "poc-mcish" } {
    txt_ids[0] = tmap.load(t040);
    txt_ids[1] = tmap.load(t101);
    txt_ids[2] = tmap.load(t131);
  }
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };
  ofs::framebuffer ofs_fb { swc.extent(), max_sampling() };

  hai::array<timing::query> tq { swc.count() };

  ext_stuff() {
    vv::as()->post.update_descriptor_sets(ofs_fb);
    vv::as()->post.setup(swc);
  }
};

static void render_to_offscreen() {
  auto cb = vv::ss()->cb.cb();
  auto ext = vv::ss()->swc.extent();

  upc pc {
    .aspect = vv::ss()->swc.aspect(),
  };

  voo::cmd_render_pass rp { vee::render_pass_begin {
    .command_buffer = cb,
    .render_pass = *vv::ss()->ofs_fb.rp,
    .framebuffer = *vv::ss()->ofs_fb.fb,
    .extent = vv::ss()->swc.extent(),
    .clear_colours { 
      vee::clear_colour({ 0, 0, 0, 1 }), 
      vee::clear_colour({ 0, 0, 10, 0 }), 
      vee::clear_colour({ 0, 0, 0, 0 }), 
      vee::clear_depth(1.0),
      vee::clear_colour({ 0, 0, 0, 1 }), 
      vee::clear_colour({ 0, 0, 10, 0 }), 
      vee::clear_colour({ 0, 0, 0, 0 }), 
      vee::clear_depth(1.0),
    },
  }, true };
  vee::cmd_set_viewport(cb, ext);
  vee::cmd_set_scissor(cb, ext);
  vee::cmd_bind_gr_pipeline(cb, *vv::as()->ofs_ppl);
  vee::cmd_push_vertex_constants(cb, *vv::as()->ofs_pl, &pc);
  vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->cube, 0);
  vee::cmd_bind_vertex_buffers(cb, 1, *vv::as()->insts, 0);
  vee::cmd_bind_descriptor_set(cb, *vv::as()->ofs_pl, 0, vv::as()->tmap.dset());
  vee::cmd_draw(cb, vv::as()->cube.count(), vv::as()->insts.count());
}

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->swc.acquire_next_image();
    auto cb = vv::ss()->cb.cb();
    auto & qp = vv::ss()->tq[vv::ss()->swc.index()];

    {
      voo::cmd_buf_one_time_submit ots { cb };

      qp.write_begin(cb);

      render_to_offscreen();

      qp.write_prepost(cb);

      vv::as()->post.render(cb, vv::ss()->swc);

      qp.write_end(cb);
    }
    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();

    static struct count {
      sitime::stopwatch w {};
      int i = 0;
      ~count() { silog::infof(">>>> %.3f", i / w.secs()); }
    } * cnt = new count {};
    cnt->i++;
    if (cnt->w.secs() > 3) {
      delete cnt;
      cnt = new count {};
    }
  });
}
