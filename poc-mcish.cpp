#pragma leco app
#pragma leco add_resource_dir assets
#pragma leco add_shader "poc-mcish.frag"
#pragma leco add_shader "poc-mcish.vert"
#pragma leco add_shader "ofs.frag"
#pragma leco add_shader "ofs.vert"

import clay;
import cube;
import dotz;
import hai;
import ofs;
import silog;
import sitime;
import sv;
import texmap;
import traits;
import vinyl;
import voo;

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

namespace inst {
  struct t {
    dotz::vec3 pos;
    float txtid;
  };

  struct buffer : clay::buffer<t> {
    buffer() : clay::buffer<t> { 128 * 128 } {
      auto m = map();
      for (auto x = 0; x < 128; x++) {
        for (auto y = 0; y < 128; y++) {
          unsigned n = (x + y) % 4;
          if (n == 3) continue;

          m += t {
            .pos { x - 64, -1, y - 64 },
            .txtid = static_cast<float>(n),
          };
        }
      }
    }
  };
}

struct app_stuff : vinyl::base_app_stuff {
  cube::buffer cube {};
  inst::buffer insts {};

  texmap::cache tmap {};
  hai::array<unsigned> txt_ids { 3 };

  vee::pipeline_layout ofs_pl = vee::create_pipeline_layout(
      tmap.dsl(),
      vee::vertex_push_constant_range<upc>());
  vee::gr_pipeline ofs_ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *ofs_pl,
    .render_pass = *ofs::render_pass(),
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .depth = vee::depth::op_less(),
    .shaders {
      *clay::vert_shader("poc-mcish", [] {}),
      *clay::frag_shader("poc-mcish", [] {}),
    },
    .bindings {
      cube::buffer::vertex_input_bind(),
      inst::buffer::vertex_input_bind_per_instance(),
    },
    .attributes { 
      vee::vertex_attribute_vec3(0, traits::offset_of(&cube::vtx::pos)),
      vee::vertex_attribute_vec2(0, traits::offset_of(&cube::vtx::uv)),
      vee::vertex_attribute_vec4(1, traits::offset_of(&inst::t::pos)),
    },
  });

  voo::single_frag_dset dset { 1 };
  vee::pipeline_layout pl = vee::create_pipeline_layout(dset.descriptor_set_layout());
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *voo::single_att_depth_render_pass(dq),
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    .depth = vee::depth::op_less(),
    .shaders {
      *clay::vert_shader("ofs", [] {}),
      *clay::frag_shader("ofs", [] {}),
    },
    .bindings {},
    .attributes {},
  });

  app_stuff() : base_app_stuff { "poc-mcish" } {
    txt_ids[0] = tmap.load(t040);
    txt_ids[1] = tmap.load(t101);
    txt_ids[2] = tmap.load(t131);
  }
};
struct ext_stuff : vinyl::base_extent_stuff {
  ofs::framebuffer ofs_fb { sw.extent() };
  vee::sampler smp = vee::create_sampler(vee::linear_sampler);

  ext_stuff() : base_extent_stuff { vv::as() } {
    vee::update_descriptor_set(vv::as()->dset.descriptor_set(), 0, *ofs_fb.colour.iv, *smp);
  }
};

static void render_to_offscreen() {
  auto cb = vv::ss()->sw.command_buffer();
  auto ext = vv::ss()->sw.extent();

  upc pc {
    .aspect = vv::ss()->aspect(),
  };

  voo::cmd_render_pass rp { vee::render_pass_begin {
    .command_buffer = cb,
    .render_pass = *vv::ss()->ofs_fb.rp,
    .framebuffer = *vv::ss()->ofs_fb.fb,
    .extent = vv::ss()->sw.extent(),
    .clear_colours { 
      vee::clear_colour({ 0, 0, 0, 1 }), 
      vee::clear_depth(1.0),
    },
  }, true };
  vee::cmd_set_viewport_flipped(cb, ext);
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
    vv::ss()->frame([] {
      render_to_offscreen();

      auto rp = vv::ss()->sw.cmd_render_pass({
        .clear_colours { 
          vee::clear_colour({}), 
          vee::clear_depth(1.0),
        },
      });

      auto cb = vv::ss()->sw.command_buffer();
      vee::cmd_set_viewport(cb, vv::ss()->sw.extent());
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vee::cmd_bind_descriptor_set(cb, *vv::as()->pl, 0, vv::as()->dset.descriptor_set());
      vee::cmd_draw(cb, vv::as()->cube.count(), vv::as()->insts.count());

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
  });
}
