export module texmap;
import hai;
import hashley;
import no;
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
    vee::descriptor_set m_dset = vee::allocate_descriptor_set(*m_dpool, *m_dsl);
    hai::array<voo::bound_image> m_imgs { max_sets };

    hashley::niamh m_ids { 127 };
    unsigned m_count = 0;

  public:
    auto load(sv name) {
      if (m_ids.has(name)) return m_ids[name];

      auto id = m_count++;
      m_ids[name] = id;
      voo::load_image(name, &m_imgs[id], [this,id](auto sz) {
        vee::update_descriptor_set(m_dset, 0, id, *m_imgs[id].iv, *m_smp);
      });
      return id;
    }

    constexpr auto dset() const { return m_dset; }
  };
}
