#include "amanita_arcade.h"

#include "aa_texture_2d.h"
#include "aa_texture_2d_fonts.ipp"


namespace aa {
  void AA_OPTIMIZE Texture2D::fill_solid(Color const & c) {
    size_t count = _width * _height;
    for(size_t i = 0; i < count; ++i) {
      _data[i] = c;
    }
  }


  void AA_OPTIMIZE Texture2D::lerp_solid(Color const & c, float a) {
    size_t count = _width * _height;
    for(size_t i = 0; i < count; ++i) {
      _data[i].lerp_this(c, a);
    }
  }

  void AA_OPTIMIZE Texture2D::box_solid(int32_t x, int32_t y,
      int32_t w, int32_t h, aa::Color const & c) {
    if((w < 0) || (h < 0)) {
      Debug::error("Texture2D::box_grad() with negative size not implemented");
      // swap colors, correct x/y,
      return;
    }
    if((x < 0) || ((size_t)w > _width) || ((size_t)(x + w) > _width) ||
        (y < 0) || ((size_t)h > _height) || ((size_t)(y + h) > _height)) {
      Debug::error("Texture2D::box_grad() with clipping not implemented");
      return;
    }

    for(size_t yi = 0; yi < (size_t)h; ++yi) {
      for(size_t xi = 0; xi < (size_t)w; ++xi) {
        set((size_t)x + xi, (size_t)y + yi, c);
      }
    }
  }


  void AA_OPTIMIZE Texture2D::box_grad_c(int32_t x, int32_t y, int32_t w,
      int32_t h, Color const & cx0y0, Color const & cx1y0, Color const & cx0y1,
      Color const & cx1y1) {
    if((w < 0) || (h < 0)) {
      Debug::error(
        "Texture2D::box_grad_o() with negative size not implemented");
      // swap colors, correct x/y,
      return;
    }
    if((x < 0) || ((size_t)w > _width) || ((size_t)(x + w) > _width) ||
        (y < 0) || ((size_t)h > _height) || ((size_t)(y + h) > _height)) {
      Debug::error("Texture2D::box_grad() with clipping not implemented");
      return;
    }

    // "closed" dy, dx calculation
    float dy = 1.0f / (h - 1);
    float dx = 1.0f / (w - 1);
    float ay = 0.0f;
    for(size_t yi = 0; yi < (size_t)h; ++yi) {
      Color cx0 = cx0y0.lerp(cx0y1, ay);
      Color cx1 = cx1y0.lerp(cx1y1, ay);
      float ax = 0.0f;
      for(size_t xi = 0; xi < (size_t)w; ++xi) {
        set((size_t)x + xi, (size_t)y + yi, cx0.lerp(cx1, ax));
        ax += dx;
      }
      ay += dy;
    }
  }


  void AA_OPTIMIZE Texture2D::box_grad_o(int32_t x, int32_t y, int32_t w,
      int32_t h, Color const & cx0y0, Color const & cx1y0, Color const & cx0y1,
      Color const & cx1y1) {
    if((w < 0) || (h < 0)) {
      Debug::error(
        "Texture2D::box_grad_c() with negative size not implemented");
      // swap colors, correct x/y,
      return;
    }
    if((x < 0) || ((size_t)w > _width) || ((size_t)(x + w) > _width) ||
        (y < 0) || ((size_t)h > _height) || ((size_t)(y + h) > _height)) {
      Debug::error("Texture2D::box_grad() with clipping not implemented");
      return;
    }

    // "open" dy, dx calculation
    float dy = 1.0f / h;
    float dx = 1.0f / w;
    float ay = 0.0f;
    for(size_t yi = 0; yi < (size_t)h; ++yi) {
      Color cx0 = cx0y0.lerp(cx0y1, ay);
      Color cx1 = cx1y0.lerp(cx1y1, ay);
      float ax = 0.0f;
      for(size_t xi = 0; xi < (size_t)w; ++xi) {
        set((size_t)x + xi, (size_t)y + yi, cx0.lerp(cx1, ax));
        ax += dx;
      }
      ay += dy;
    }
  }


  void AA_OPTIMIZE Texture2D::box_mask(int32_t x, int32_t y,
      int32_t w, int32_t h, Texture2D const * tex) {
    if((w < 0) || (h < 0)) {
      Debug::error("Texture2D::box_mask() with negative size not implemented");
      // swap colors, correct x/y,
      return;
    }
    if((x < 0) || ((size_t)w > _width) || ((size_t)(x + w) > _width) ||
        (y < 0) || ((size_t)h > _height) || ((size_t)(y + h) > _height)) {
      Debug::error("Texture2D::box_grad() with clipping not implemented");
      return;
    }
    if(((size_t)w > tex->get_width()) || ((size_t)h > tex->get_height())) {
      Debug::error("Texture2D::box_grad() with clamping not implemented");
      return;
    }

    for(size_t yi = 0; yi < (size_t)h; ++yi) {
      memcpy(_data + _width * (y + yi) + x, tex->_data + tex->_width * yi,
        sizeof (Color) * w);
    }
  }


  void AA_OPTIMIZE Texture2D::copy(Texture2D const * src, size_t x_ofs,
      size_t y_ofs) {
    if((_width + x_ofs > src->_width) || (_height + y_ofs > src->_height)) {
      // with clipping
      Debug::error("Texture2D::copy() with clipping not implemented");
      return;
    }

    size_t x = 0;
    size_t y = 0;
    size_t w = _width;
    size_t h = _height;
    size_t dest_stride = get_stride();
    Color * dest_data = _data + dest_stride * y + x;
    size_t src_stride = src->get_stride();
    Color const * src_data = src->get_data() + src_stride * y_ofs + x_ofs;
    for(size_t yi = 0; yi < h; ++yi) {
      memcpy(dest_data, src_data, w * sizeof (Color));
      dest_data += dest_stride;
      src_data += src_stride;
    }
  }


  void AA_OPTIMIZE Texture2D::mix(Texture2D const * src, size_t x_ofs,
      size_t y_ofs) {
    if((_width + x_ofs > src->_width) || (_height + y_ofs > src->_height)) {
      // with clipping
      Debug::error("Texture2D::mix() with clipping not implemented");
      return;
    }

    size_t x = 0;
    size_t y = 0;
    size_t w = _width;
    size_t h = _height;
    size_t dest_stride = get_stride();
    Color * dest_data = _data + dest_stride * y + x;
    size_t src_stride = src->get_stride();
    Color const * src_data = src->get_data() + src_stride * y_ofs + x_ofs;
    for(size_t yi = 0; yi < h; ++yi) {
      for(size_t xi = 0; xi < w; ++xi) {
        dest_data[xi].mix_this(src_data[xi]);
      }
      dest_data += dest_stride;
      src_data += src_stride;
    }
  }


  void AA_OPTIMIZE Texture2D::lerp(Texture2D const * src, float a,
      size_t x_ofs, size_t y_ofs) {
    if((_width + x_ofs > src->_width) || (_height + y_ofs > src->_height)) {
      // with clipping
      Debug::error("Texture2D::lerp() with clipping not implemented");
      return;
    }

    size_t x = 0;
    size_t y = 0;
    size_t w = _width;
    size_t h = _height;
    size_t dest_stride = get_stride();
    Color * dest_data = _data + dest_stride * y + x;
    size_t src_stride = src->get_stride();
    Color const * src_data = src->get_data() + src_stride * y_ofs + x_ofs;
    for(size_t yi = 0; yi < h; ++yi) {
      for(size_t xi = 0; xi < w; ++xi) {
        dest_data[xi].lerp_this(src_data[xi], a);
      }
      dest_data += dest_stride;
      src_data += src_stride;
    }
  }


  void AA_OPTIMIZE Texture2D::bubble_x(float px, float radius,
      Color const & c) {
    float scale = 1.0f / radius;
    size_t i = 0;
    for(size_t y = 0; y < _height; ++y) {
      for(size_t x = 0; x < _width; ++x) {
        float delta = fabsf(px - x) * scale;
        if(delta < 1.0f) {
          _data[i].lerp_this(c, 1.0f - delta);
        }
        ++i;
      }
    }
  }


  void AA_OPTIMIZE Texture2D::char_5x5_solid(int32_t px, int32_t py, char ch,
      bool invert, Color const & c) {
    if((px < 0) || ((size_t)px + 5 > _width) ||
        (py < 0) || ((size_t)py + 5 > _height)) {
      // with clipping
      Debug::error(
        "Texture2D::char_5x5_solid() with clipping not implemented");
      return;
    }
    if((ch < 32) || (ch >= 128)) {
      if(invert) {
        box_solid(px, py, 5, 5, c);
      }
      return;
    }

    uint8_t const * glyph = font_5x5[ch - 32];
    uint_fast8_t invert_mask = invert ? ~0 : 0;
    for(size_t yi = 0; yi < 5; ++yi) {
      uint_fast8_t row = glyph[yi] ^ invert_mask;
      for(size_t xi = 0; xi < 5; ++xi) {
        if(row & (0b10000 >> xi)) {
          _data[((size_t)py + yi) * _width + ((size_t)px + xi)] = c;
        }
      }
    }
  }

  void AA_OPTIMIZE Texture2D::char_5x5_solid(int32_t px, int32_t py,
      char const * str, Color const & c) {
    int32_t x = px;
    for(char const * p = str; *p; ++p) {
      char_5x5_solid(x, py, *p, false, c);
      x += 5;
    }
  }

  void AA_OPTIMIZE Texture2D::char_5x5_mask(int32_t px, int32_t py, char ch,
      bool invert, Texture2D const * tex) {
    if((px < 0) || ((size_t)px + 5 > _width) ||
        (py < 0) || ((size_t)py + 5 > _height)) {
      // with clipping
      Debug::error("Texture2D::char_5x5_mask() with clipping not implemented");
      return;
    }
    if((tex->get_width() < 5) || (tex->get_height() < 5)) {
      Debug::error(
        "Texture2D::char_5x5_mask() with clamping not implemented");
      return;
    }
    if((ch < 32) || (ch >= 128)) {
      if(invert) {
        box_mask(px, py, 5, 5, tex);
      }
      return;
    }

    uint8_t const * glyph = font_5x5[ch - 32];
    uint_fast8_t invert_mask = invert ? ~0 : 0;
    for(size_t yi = 0; yi < 5; ++yi) {
      uint_fast8_t row = glyph[yi] ^ invert_mask;
      for(size_t xi = 0; xi < 5; ++xi) {
        if(row & (0b10000 >> xi)) {
          _data[((size_t)py + yi) * _width + ((size_t)px + xi)] =
            tex->get(xi, yi);
        }
      }
    }
  }


  void AA_OPTIMIZE Texture2D::char_5x5_mask(int32_t px, int32_t py,
      char const * str, Texture2D const * tex) {
    int32_t x = px;
    for(char const * p = str; *p; ++p) {
      char_5x5_mask(x, py, *p, false, tex);
      x += 5;
    }
  }


  void AA_OPTIMIZE Texture2D::char_10x15_solid(int32_t px, int32_t py, char ch,
      bool invert, Color const & c) {
    if((px < 0) || ((size_t)px + 5 > _width) ||
        (py < 0) || ((size_t)py + 5 > _height)) {
      // with clipping
      Debug::error(
        "Texture2D::char_10x15_solid() with clipping not implemented");
      return;
    }
    if(ch < '0' || ch > '9') {
      if(invert) {
        box_solid(px, py, 10, 15, c);
      }
      return;
    }

    uint16_t const * glyph = font_10x15_numerals[ch - '0'];
    uint_fast16_t invert_mask = invert ? ~0 : 0;
    for(size_t yi = 0; yi < 15; ++yi) {
      uint_fast16_t row = glyph[yi] ^ invert_mask;
      for(size_t xi = 0; xi < 10; ++xi) {
        if(row & (0b1000000000 >> xi)) {
          _data[((size_t)py + yi) * _width + ((size_t)px + xi)] = c;
        }
      }
    }
  }


  void AA_OPTIMIZE Texture2D::char_10x15_solid(int32_t px, int32_t py,
      char const * str, Color const & c) {
    int32_t x = px;
    for(char const * p = str; *p; ++p) {
      char_10x15_solid(x, py, *p, false, c);
      x += 10;
    }
  }


  void AA_OPTIMIZE Texture2D::char_10x15_mask(int32_t px, int32_t py, char ch,
      bool invert, Texture2D const * tex) {
    if((px < 0) || ((size_t)px + 5 > _width) ||
        (py < 0) || ((size_t)py + 5 > _height)) {
      // with clipping
      Debug::error(
        "Texture2D::char_10x15_mask() with clipping not implemented");
      return;
    }
    if((tex->get_width() < 10) || (tex->get_height() < 15)) {
      Debug::error(
        "Texture2D::char_10x15_mask() with clamping not implemented");
      return;
    }
    if(ch < '0' || ch > '9') {
      if(invert) {
        box_mask(px, py, 10, 15, tex);
      }
      return;
    }

    uint16_t const * glyph = font_10x15_numerals[ch - '0'];
    uint_fast16_t invert_mask = invert ? ~0 : 0;
    for(size_t yi = 0; yi < 15; ++yi) {
      uint_fast16_t row = glyph[yi] ^ invert_mask;
      for(size_t xi = 0; xi < 10; ++xi) {
        if(row & (0b1000000000 >> xi)) {
          _data[((size_t)py + yi) * _width + ((size_t)px + xi)] =
            tex->get(xi, yi);
        }
      }
    }
  }


  void AA_OPTIMIZE Texture2D::char_10x15_mask(int32_t px, int32_t py,
      char const * str, Texture2D const * tex) {
    int32_t x = px;
    for(char const * p = str; *p; ++p) {
      char_10x15_mask(x, py, *p, false, tex);
      x += 10;
    }
  }


  void AA_OPTIMIZE Texture2D::write_grb_color32(uint32_t * dest,
      size_t dest_w, size_t dest_h, bool initial_invert_x,
      size_t src_x, size_t src_y) const {
    //Debug::tracef("w: %p %2lu %2lu %c %2lu %2lu", dest, dest_w, dest_h,
    //  initial_invert_x ? 'y' : 'n', src_x, src_y);
    ptrdiff_t p_inc = initial_invert_x ? -1 : 1;
    for(size_t yi = 0; yi < dest_h; ++yi) {
      uint32_t * p = dest + ((p_inc < 0) ? (dest_w - 1) : 0) + dest_w * yi;
      for(size_t xi = 0; xi < dest_w; ++xi) {
        *p = get(src_x + xi, src_y + yi).to_grb_color32();
        p += p_inc;
      }
      p_inc = -p_inc;
    }
  }


  void AA_OPTIMIZE Texture2D::write_brg_color32(uint32_t * dest,
      size_t dest_w, size_t dest_h, bool initial_invert_x,
      size_t src_x, size_t src_y) const {
    //Debug::tracef("w: %p %2lu %2lu %c %2lu %2lu", dest, dest_w, dest_h,
    //  initial_invert_x ? 'y' : 'n', src_x, src_y);
    ptrdiff_t p_inc = initial_invert_x ? -1 : 1;
    for(size_t yi = 0; yi < dest_h; ++yi) {
      uint32_t * p = dest + ((p_inc < 0) ? (dest_w - 1) : 0) + dest_w * yi;
      for(size_t xi = 0; xi < dest_w; ++xi) {
        *p = get(src_x + xi, src_y + yi).to_brg_color32();
        p += p_inc;
      }
      p_inc = -p_inc;
    }
  }
}
