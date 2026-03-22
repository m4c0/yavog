module;
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

export module texmap;
import hai;
import hay;
import hashley;
import jute;
import no;
import silog;
import sires;
import sv;
import voo;

namespace texmap {
  static constexpr const auto max_sets = 128;

  export auto descriptor_set_layout() {
    return vee::create_descriptor_set_layout({
      vee::dsl_fragment_sampler(max_sets)
    });
  }

  export class cache : no::no {
    vee::sampler m_smp = vee::create_sampler(vee::linear_sampler);

    vee::descriptor_set_layout m_dsl = descriptor_set_layout();
    vee::descriptor_pool m_dpool = vee::create_descriptor_pool(1, {
      vee::combined_image_sampler(max_sets)
    });
    vee::descriptor_set m_dset = voo::allocate_descriptor_set("texmap", *m_dpool, *m_dsl);
    hai::array<voo::bound_image> m_imgs { max_sets };

    hashley::niamh m_ids { 127 };
    unsigned m_count = 0;

    void load_packed_image(sv name, unsigned id);

  public:
    auto load(sv name) {
      if (m_ids.has(name)) return m_ids[name];

      auto id = m_count++;
      m_ids[name] = id;
      if (name.ends_with(".pci")) {
        load_packed_image(name, id);
      } else {
        voo::load_image(name, &m_imgs[id], [this,id](auto sz) {
          vee::update_descriptor_set(m_dset, 0, id, *m_imgs[id].iv, *m_smp);
        });
      }
      return id;
    }

    constexpr auto dset() const { return m_dset; }
  };
}

module : private;

static auto open(sv file) {
  auto f = fopen(sires::real_path_name(file).begin(), "rb");
  if (!f) silog::die("missing resource file");
  return f;
}
static auto read_u32(FILE * f) {
  unsigned r {};
  if (1 != fread(&r, 4, 1, f)) silog::die("error reading file");
  return r;
}
void texmap::cache::load_packed_image(sv file, unsigned id) {
  silog::info(jute::fmt<"Loading packed image %s">(file));

  hay<FILE *, open, fclose> f { file };
  if ('PCIm' != read_u32(f)) silog::die("invalid file format");
  unsigned w = read_u32(f);
  unsigned h = read_u32(f);
  unsigned sz = w * h * 2;

  auto host = voo::bound_buffer::create_from_host(sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
  {
    voo::mapmem c { *host.memory };
    if (1 != fread(*c, sz, 1, f)) silog::die("error reading image data");
  }

  constexpr const auto fmt = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
  constexpr const auto usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  VkExtent2D ext { w, h };
  auto & bi = m_imgs[id];
  bi.img = vee::create_image({w, h}, fmt, usage); 
  bi.mem = vee::create_local_image_memory(physical_device(), *bi.img);
  vee::bind_image_memory(*bi.img, *bi.mem);
  bi.iv = vee::create_image_view(*bi.img, fmt);

  voo::fence fc { false };
  voo::command_pool cpool {};
  auto cb = cpool.allocate_primary_command_buffer();

  {
    voo::cmd_buf_one_time_submit ots { cb };
    vee::cmd_pipeline_barrier(cb, *bi.img, vee::from_host_to_transfer);
    vee::cmd_copy_buffer_to_image(cb, ext, *host.buffer, *bi.img);
    vee::cmd_pipeline_barrier(cb, *bi.img, vee::from_transfer_to_fragment);
  }
  voo::queue::universal()->queue_submit({
    .fence = fc,
    .command_buffer = cb,
  });

  fc.wait();

  vee::update_descriptor_set(m_dset, 0, id, *m_imgs[id].iv, *m_smp);
}

