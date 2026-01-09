#pragma leco app
#pragma leco add_resource_dir assets
#pragma leco add_shader "poc-mcish.frag"
#pragma leco add_shader "poc-mcish.vert"

import clay;
import cube;
import dotz;
import silog;
import sitime;
import traits;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct upc {
  float aspect;
  float fov = 90;
};

namespace inst {
  struct t {
    dotz::vec3 pos;
    float _pad;
  };

  struct buffer : clay::buffer<t> {
    buffer() : clay::buffer<t> { 128 * 128 } {
      auto m = map();
      for (auto x = 0; x < 128; x++) {
        for (auto y = 0; y < 128; y++) {
          m += t { .pos { x - 64, -1, y - 64 } };
        }
      }
    }
  };
}

struct app_stuff : vinyl::base_app_stuff {
  cube::buffer cube {};
  inst::buffer insts {};

  voo::single_frag_dset dset { 1 };

  vee::render_pass rp = voo::single_att_depth_render_pass(dq);
  vee::pipeline_layout pl = vee::create_pipeline_layout(
      dset.descriptor_set_layout(),
      vee::vertex_push_constant_range<upc>());
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
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
      vee::vertex_attribute_vec3(1, traits::offset_of(&inst::t::pos)),
    },
  });

  vee::sampler smp = vee::create_sampler(vee::nearest_sampler);
  voo::bound_image t040 {};

  app_stuff() : base_app_stuff { "poc-mcish" } {
    voo::load_image("Tiles040_1K-JPG_Color.jpg", &t040, [this](auto sz) {
      vee::update_descriptor_set(dset.descriptor_set(), 0, *t040.iv, *smp);
    });
  }
};
struct ext_stuff : vinyl::base_extent_stuff {
  ext_stuff() : base_extent_stuff { vv::as() } {}
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->frame([] {
      [[maybe_unused]] auto rp = vv::ss()->clear({ 0, 0, 0, 1 });

      upc pc {
        .aspect = vv::ss()->aspect(),
      };

      auto cb = vv::ss()->sw.command_buffer();
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vee::cmd_push_vertex_constants(cb, *vv::as()->pl, &pc);
      vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->cube, 0);
      vee::cmd_bind_vertex_buffers(cb, 1, *vv::as()->insts, 0);
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
