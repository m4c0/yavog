#pragma leco app
#pragma leco add_shader "poc-model.frag"
#pragma leco add_shader "poc-model.vert"
import casein;
import clay;
import cube;
import dotz;
import hai;
import ofs;
import prism;
import sitime;
import traits;
import vinyl;
import voo;
import wagen;

using namespace wagen;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct upc {
  dotz::vec2 angles;
  float explode;
  float uv_mix;
};

static inline auto create_dq() {
  auto nxt = vee::physical_device_extended_dynamic_state({
    .extendedDynamicState3PolygonMode = true,
  });
  return voo::device_and_queue { "poc-model", casein::native_ptr, {
    .feats {
      .fillModeNonSolid = true,
      .samplerAnisotropy = true,
    },
    .next = &nxt,
  }};
}

struct app_stuff {
  voo::device_and_queue dq = create_dq();
  prism::v_buffer vtx {};
  prism::ix_buffer idx {};

  vee::render_pass rp = voo::single_att_render_pass(dq);

  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());

  vee::gr_pipeline dots_ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .polygon_mode = VK_POLYGON_MODE_POINT,
    .shaders {
      voo::vert_shader("poc-model.vert.spv").pipeline_stage(),
      voo::frag_shader("poc-model.frag.spv").pipeline_stage(),
    },
    .bindings {
      vee::vertex_input_bind(sizeof(ofs::vtx)),
    },
    .attributes {
      vee::vertex_attribute_vec4(0, traits::offset_of(&ofs::vtx::pos)),
      vee::vertex_attribute_vec3(0, traits::offset_of(&ofs::vtx::normal)),
      vee::vertex_attribute_vec2(0, traits::offset_of(&ofs::vtx::uv)),
    },
  });
  vee::gr_pipeline faces_ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .polygon_mode = vee::dynamic_polygon_mode,
    .shaders {
      voo::vert_shader("poc-model.vert.spv").pipeline_stage(),
      voo::frag_shader("poc-model.frag.spv").pipeline_stage(),
    },
    .bindings {
      vee::vertex_input_bind(sizeof(ofs::vtx)),
    },
    .attributes {
      vee::vertex_attribute_vec4(0, traits::offset_of(&ofs::vtx::pos)),
      vee::vertex_attribute_vec3(0, traits::offset_of(&ofs::vtx::normal)),
      vee::vertex_attribute_vec2(0, traits::offset_of(&ofs::vtx::uv)),
    },
  });
};
struct ext_stuff {
  voo::single_cb cb {};
  voo::swapchain swc { vv::as()->dq, false };
  hai::array<vee::framebuffer> fbs = swc.create_framebuffers(*vv::as()->rp);

  ext_stuff() {}
};

static upc g_pc {};
extern "C" void casein_init() {
  casein::window_size = { 600, 600 };

  vv::setup([] {
    auto ext = vv::ss()->swc.extent();

    if (g_pc.angles.y < -60) g_pc.angles.y = -60;
    if (g_pc.angles.y >  60) g_pc.angles.y =  60;

    vv::ss()->swc.acquire_next_image();

    auto cb = vv::ss()->cb.cb();
    {
      voo::cmd_buf_one_time_submit ots { cb };
      voo::cmd_render_pass rp { vee::render_pass_begin {
        .command_buffer = cb,
        .render_pass = *vv::as()->rp,
        .framebuffer = *vv::ss()->fbs[vv::ss()->swc.index()],
        .extent = ext,
        .clear_colours { 
          vee::clear_colour({ 0, 0, 0, 1 }),
        },
      }, true };
      vee::cmd_set_viewport(cb, ext);
      vee::cmd_set_scissor(cb, ext);
      vee::cmd_push_vertex_constants(cb, *vv::as()->pl, &g_pc);
      vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->vtx, 0);
      vee::cmd_bind_index_buffer_u16(cb, *vv::as()->idx);
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->faces_ppl);

      vkCmdSetPolygonModeEXT(cb, VK_POLYGON_MODE_FILL);
      vee::cmd_draw_indexed(cb, vv::as()->idx.count());

      upc pc = g_pc;
      pc.explode *= 0.3;
      vee::cmd_push_vertex_constants(cb, *vv::as()->pl, &pc);
      vkCmdSetPolygonModeEXT(cb, VK_POLYGON_MODE_LINE);
      vee::cmd_draw_indexed(cb, vv::as()->idx.count());

      vee::cmd_bind_gr_pipeline(cb, *vv::as()->dots_ppl);
      vee::cmd_draw_indexed(cb, vv::as()->idx.count());
    }

    vv::ss()->swc.queue_submit(cb);
    vv::ss()->swc.queue_present();
  });

  using namespace casein;
  handle(KEY_DOWN, K_A, [] { g_pc.angles.x -= 10; });
  handle(KEY_DOWN, K_D, [] { g_pc.angles.x += 10; });
  handle(KEY_DOWN, K_W, [] { g_pc.angles.y -= 5; });
  handle(KEY_DOWN, K_S, [] { g_pc.angles.y += 5; });

  handle(KEY_DOWN, K_LEFT,  [] { g_pc.angles.x -= 10; });
  handle(KEY_DOWN, K_RIGHT, [] { g_pc.angles.x += 10; });
  handle(KEY_DOWN, K_UP,    [] { g_pc.angles.y -= 5; });
  handle(KEY_DOWN, K_DOWN,  [] { g_pc.angles.y += 5; });

  handle(KEY_DOWN, K_SPACE, [] { g_pc.explode = 0.2 - g_pc.explode; });
  handle(KEY_DOWN, K_U, [] { g_pc.uv_mix = 1 - g_pc.uv_mix; });

  window_title = "poc-model";
}
