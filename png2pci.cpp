#pragma leco test
#include <stdio.h>

import jojo;
import jute;
import hay;
import stubby;
import sv;
import wagen;

using namespace wagen;
using namespace jute::literals;

static void write_u16(auto & f, unsigned id) {
  if (1 != fwrite(&id, 2, 1, f)) throw 0;
}
static void write_u32(auto & f, unsigned id) {
  if (1 != fwrite(&id, 4, 1, f)) throw 0;
}

static void conv(sv name) {
  auto img = stbi::load(jojo::slurp(name));
  auto [fn,ext] = name.rsplit('.');
  auto out = (fn + ".pci"_s).cstr();
  
  hay<FILE *, fopen, fclose> f { out.begin(), "wb" };
  write_u32(f, 'PCIm');
  write_u32(f, img.width);
  write_u32(f, img.height);

  unsigned char * c = *img.data;
  for (auto i = 0; i < img.width * img.height; i++) {
    unsigned short a = *c++ ? 1 : 0;
    unsigned short b = *c++ >> 3;
    unsigned short g = *c++ >> 3;
    unsigned short r = *c++ >> 3;
    write_u16(f,
        a << 15 |
        b << 10 |
        g << 5 |
        r);
  }
}

int main() {
  conv("assets/Tiles101_1K-JPG_Color.jpg");
}
