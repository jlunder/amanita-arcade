#include "amanita_arcade.h"

#include "aa_texture_2d.h"


namespace aa {
  namespace {
    uint16_t font_10x15_numerals[10][15] = {
      { // '0'
        0b0000000000,
        0b0001111000,
        0b0010000100,
        0b0010000100,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0010000100,
        0b0010000100,
        0b0001111000,
        0b0000000000,
      },
      { // '1'
        0b0000000000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000010000,
        0b0000000000,
      },
      { // '2'
        0b0000000000,
        0b0001111000,
        0b0010000100,
        0b0100000010,
        0b0100000010,
        0b0000000010,
        0b0000000100,
        0b0000111000,
        0b0001000000,
        0b0010000000,
        0b0100000000,
        0b0100000000,
        0b0100000000,
        0b0111111110,
        0b0000000000,
      },
      { // '3'
        0b0000000000,
        0b0111111110,
        0b0000000010,
        0b0000000100,
        0b0000001000,
        0b0000011000,
        0b0000000100,
        0b0000000010,
        0b0000000010,
        0b0000000010,
        0b0000000010,
        0b0100000010,
        0b0010000100,
        0b0001111000,
        0b0000000000,
      },
      { // '4'
        0b0000000000,
        0b0000001000,
        0b0000011000,
        0b0000101000,
        0b0000101000,
        0b0001001000,
        0b0010001000,
        0b0010001000,
        0b0100001000,
        0b0111111110,
        0b0000001000,
        0b0000001000,
        0b0000001000,
        0b0000001000,
        0b0000000000,
      },
      { // '5'
        0b0000000000,
        0b0011111100,
        0b0010000000,
        0b0010000000,
        0b0100000000,
        0b0100111000,
        0b0111000100,
        0b0100000010,
        0b0000000010,
        0b0000000010,
        0b0000000010,
        0b0100000010,
        0b0010000100,
        0b0001111000,
        0b0000000000,
      },
      { // '6'
        0b0000000000,
        0b0000010000,
        0b0000100000,
        0b0000100000,
        0b0001000000,
        0b0001000000,
        0b0011111000,
        0b0010000100,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0010000100,
        0b0001111000,
        0b0000000000,
      },
      { // '7'
        0b0000000000,
        0b0111111110,
        0b0000000010,
        0b0000000010,
        0b0000000100,
        0b0000000100,
        0b0000001000,
        0b0000001000,
        0b0000001000,
        0b0000010000,
        0b0000010000,
        0b0000100000,
        0b0000100000,
        0b0000100000,
        0b0000000000,
      },
      { // '8'
        0b0000000000,
        0b0001111000,
        0b0010000100,
        0b0010000100,
        0b0010000100,
        0b0010000100,
        0b0001111000,
        0b0010000100,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0010000100,
        0b0001111000,
        0b0000000000,
      },
      { // '9'        0b0001111000,

        0b0000000000,
        0b0001111000,
        0b0010000100,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0100000010,
        0b0010000100,
        0b0001111100,
        0b0000001000,
        0b0000001000,
        0b0000010000,
        0b0000010000,
        0b0000100000,
        0b0000000000,
      },
    };
    uint8_t font_5x5[96][5] = {
      { // ' '
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '!'
        0b01000,
        0b01000,
        0b01000,
        0b00000,
        0b01000,
      },
      { // '"'
        0b01010,
        0b01010,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '#'
        0b01010,
        0b11111,
        0b01010,
        0b11111,
        0b01010,
      },
      { // '$'
        0b01111,
        0b10100,
        0b01110,
        0b00101,
        0b11110,
      },
      { // '%'
        0b11001,
        0b10010,
        0b00100,
        0b01001,
        0b10011,
      },
      { // '&'
        0b01000,
        0b11000,
        0b01011,
        0b10100,
        0b01111,
      },
      { // '''
        0b00100,
        0b00100,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '('
        0b00100,
        0b01000,
        0b01000,
        0b01000,
        0b00100,
      },
      { // ')'
        0b01000,
        0b00100,
        0b00100,
        0b00100,
        0b01000,
      },
      { // '*'
        0b00100,
        0b10101,
        0b01110,
        0b10101,
        0b00100,
      },
      { // '+'
        0b00100,
        0b00100,
        0b11111,
        0b00100,
        0b00100,
      },
      { // ','
        0b00000,
        0b00000,
        0b00000,
        0b00100,
        0b00100,
      },
      { // '-'
        0b00000,
        0b00000,
        0b11111,
        0b00000,
        0b00000,
      },
      { // '.'
        0b00000,
        0b00000,
        0b00000,
        0b01100,
        0b01100,
      },
      { // '/'
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b10000,
      },
      { // '0'
        0b01110,
        0b10001,
        0b10101,
        0b10001,
        0b01110,
      },
      { // '1'
        0b00100,
        0b01100,
        0b00100,
        0b00100,
        0b01110,
      },
      { // '2'
        0b11110,
        0b00001,
        0b01110,
        0b10000,
        0b11111,
      },
      { // '3'
        0b11110,
        0b00001,
        0b00110,
        0b00001,
        0b11110,
      },
      { // '4'
        0b10010,
        0b10010,
        0b11111,
        0b00010,
        0b00010,
      },
      { // '5'
        0b11111,
        0b10000,
        0b11110,
        0b00001,
        0b11110,
      },
      { // '6'
        0b00110,
        0b01000,
        0b11110,
        0b10001,
        0b01110,
      },
      { // '7'
        0b11111,
        0b00001,
        0b00010,
        0b00100,
        0b00100,
      },
      { // '8'
        0b01110,
        0b10001,
        0b01110,
        0b10001,
        0b01110,
      },
      { // '9'
        0b01110,
        0b10001,
        0b01111,
        0b00001,
        0b01110,
      },
      { // ':'
        0b00000,
        0b00100,
        0b00000,
        0b00100,
        0b00000,
      },
      { // ';'
        0b00000,
        0b00100,
        0b00000,
        0b00100,
        0b00100,
      },
      { // '<'
        0b00010,
        0b00100,
        0b01000,
        0b00100,
        0b00010,
      },
      { // '='
        0b00000,
        0b01110,
        0b00000,
        0b01110,
        0b00000,
      },
      { // '>'
        0b01000,
        0b00100,
        0b00010,
        0b00100,
        0b01000,
      },
      { // '?'
        0b11110,
        0b00001,
        0b00110,
        0b00000,
        0b00100,
      },
      { // '@'
        0b01110,
        0b10111,
        0b10111,
        0b10000,
        0b01110,
      },
      { // 'A'
        0b00100,
        0b01010,
        0b11111,
        0b10001,
        0b10001,
      },
      { // 'B'
        0b11110,
        0b10001,
        0b11110,
        0b10001,
        0b11110,
      },
      { // 'C'
        0b01111,
        0b10000,
        0b10000,
        0b10000,
        0b01111,
      },
      { // 'D'
        0b11100,
        0b10010,
        0b10001,
        0b10010,
        0b11100,
      },
      { // 'E'
        0b11111,
        0b10000,
        0b11110,
        0b10000,
        0b11111,
      },
      { // 'F'
        0b11111,
        0b10000,
        0b11110,
        0b10000,
        0b10000,
      },
      { // 'G'
        0b01111,
        0b10000,
        0b10011,
        0b10001,
        0b01111,
      },
      { // 'H'
        0b10001,
        0b10001,
        0b11111,
        0b10001,
        0b10001,
      },
      { // 'I'
        0b01110,
        0b00100,
        0b00100,
        0b00100,
        0b01110,
      },
      { // 'J'
        0b00111,
        0b00001,
        0b00001,
        0b10001,
        0b01110,
      },
      { // 'K'
        0b10001,
        0b10010,
        0b11100,
        0b10010,
        0b10001,
      },
      { // 'L'
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b11111,
      },
      { // 'M'
        0b10001,
        0b11011,
        0b10101,
        0b10001,
        0b10001,
      },
      { // 'N'
        0b10001,
        0b11001,
        0b10101,
        0b10011,
        0b10001,
      },
      { // 'O'
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b01110,
      },
      { // 'P'
        0b11110,
        0b10001,
        0b11110,
        0b10000,
        0b10000,
      },
      { // 'Q'
        0b01110,
        0b10001,
        0b10001,
        0b10011,
        0b01111,
      },
      { // 'R'
        0b11110,
        0b10001,
        0b11110,
        0b10010,
        0b10001,
      },
      { // 'S'
        0b01111,
        0b10000,
        0b01110,
        0b00001,
        0b11110,
      },
      { // 'T'
        0b11111,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
      },
      { // 'U'
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01110,
      },
      { // 'V'
        0b10001,
        0b10001,
        0b10001,
        0b01010,
        0b00100,
      },
      { // 'W'
        0b10001,
        0b10001,
        0b10101,
        0b11011,
        0b10001,
      },
      { // 'X'
        0b10001,
        0b01010,
        0b00100,
        0b01010,
        0b10001,
      },
      { // 'Y'
        0b10001,
        0b01010,
        0b00100,
        0b00100,
        0b00100,
      },
      { // 'Z'
        0b11111,
        0b00010,
        0b00100,
        0b01000,
        0b11111,
      },
      { // '['
        0b01110,
        0b01000,
        0b01000,
        0b01000,
        0b01110,
      },
      { // '\'
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
      },
      { // ']'
        0b01110,
        0b00010,
        0b00010,
        0b00010,
        0b01110,
      },
      { // '^'
        0b00100,
        0b01010,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '_'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b11111,
      },
      { // '`'
        0b01000,
        0b00100,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'a'
        0b00000,
        0b00000,
        0b01111,
        0b10001,
        0b01111,
      },
      { // 'b'
        0b10000,
        0b10000,
        0b11110,
        0b10001,
        0b11110,
      },
      { // 'c'
        0b00000,
        0b00000,
        0b01111,
        0b10000,
        0b01111,
      },
      { // 'd'
        0b00001,
        0b00001,
        0b01111,
        0b10001,
        0b01111,
      },
      { // 'e'
        0b00000,
        0b01111,
        0b11111,
        0b10000,
        0b01111,
      },
      { // 'f'
        0b00110,
        0b01000,
        0b11100,
        0b01000,
        0b01000,
      },
      { // 'g'
        0b00000,
        0b01111,
        0b11111,
        0b00001,
        0b11110,
      },
      { // 'h'
        0b10000,
        0b10000,
        0b11110,
        0b10001,
        0b10001,
      },
      { // 'i'
        0b00100,
        0b00000,
        0b00100,
        0b00100,
        0b00100,
      },
      { // 'j'
        0b00010,
        0b00000,
        0b00010,
        0b00010,
        0b01100,
      },
      { // 'k'
        0b10000,
        0b10010,
        0b11100,
        0b10010,
        0b10001,
      },
      { // 'l'
        0b01100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
      },
      { // 'm'
        0b00000,
        0b00000,
        0b11010,
        0b10101,
        0b10101,
      },
      { // 'n'
        0b00000,
        0b00000,
        0b11110,
        0b10001,
        0b10001,
      },
      { // 'o'
        0b00000,
        0b00000,
        0b01110,
        0b10001,
        0b01110,
      },
      { // 'p'
        0b00000,
        0b11110,
        0b10001,
        0b11110,
        0b10000,
      },
      { // 'q'
        0b00000,
        0b01111,
        0b10001,
        0b01111,
        0b00001,
      },
      { // 'r'
        0b00000,
        0b00000,
        0b11111,
        0b10000,
        0b10000,
      },
      { // 's'
        0b00000,
        0b01111,
        0b11110,
        0b00001,
        0b11110,
      },
      { // 't'
        0b01000,
        0b11100,
        0b01000,
        0b01000,
        0b00110,
      },
      { // 'u'
        0b00000,
        0b00000,
        0b10001,
        0b10001,
        0b01111,
      },
      { // 'v'
        0b00000,
        0b00000,
        0b10001,
        0b01010,
        0b00100,
      },
      { // 'w'
        0b00000,
        0b00000,
        0b10101,
        0b10101,
        0b01010,
      },
      { // 'x'
        0b00000,
        0b00000,
        0b11011,
        0b00100,
        0b11011,
      },
      { // 'y'
        0b00000,
        0b10001,
        0b01001,
        0b00110,
        0b01100,
      },
      { // 'z'
        0b00000,
        0b11111,
        0b00110,
        0b01000,
        0b11111,
      },
      { // '{'
        0b00110,
        0b00100,
        0b01000,
        0b00100,
        0b00110,
      },
      { // '|'
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
      },
      { // '}'
        0b01100,
        0b00100,
        0b00010,
        0b00100,
        0b01100,
      },
      { // '~'
        0b00000,
        0b01010,
        0b10100,
        0b00000,
        0b00000,
      },
    };
  }


  void AA_OPTIMIZE Texture2D::fill_solid(Color c) {
    size_t count = _width * _height;
    for(size_t i = 0; i < count; ++i) {
      _data[i] = c;
    }
  }


  void AA_OPTIMIZE Texture2D::lerp_solid(Color c, float a) {
    size_t count = _width * _height;
    for(size_t i = 0; i < count; ++i) {
      _data[i].lerp_this(c, a);
    }
  }

  void AA_OPTIMIZE Texture2D::box_solid(int32_t x, int32_t y,
      int32_t w, int32_t h, aa::Color c) {
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


  void AA_OPTIMIZE Texture2D::box_grad(int32_t x, int32_t y, int32_t w,
      int32_t h, Color cx0y0, Color cx1y0, Color cx0y1, Color cx1y1) {
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


  void AA_OPTIMIZE Texture2D::copy(Texture2D const * src) {
    if((_width == src->_width) && (_height == src->_height)) {
      size_t count = _width * _height;
      memcpy(_data, src->_data, count * sizeof (Color));
    } else {
      // with clipping
      Debug::error("Texture2D::copy() with clipping not implemented");
    }
  }


  void AA_OPTIMIZE Texture2D::mix(Texture2D const * src) {
    if((_width == src->_width) && (_height == src->_height)) {
      size_t count = _width * _height;
      for(size_t i = 0; i < count; ++i) {
        _data[i].mix_this(src->_data[i]);
      }
    } else {
      // with clipping
      Debug::error("Texture2D::mix() with clipping not implemented");
    }
  }


  void AA_OPTIMIZE Texture2D::lerp(Texture2D const * other, float a) {
    if((_width == other->_width) && (_height == other->_height)) {
      size_t count = _width * _height;
      for(size_t i = 0; i < count; ++i) {
        _data[i].lerp_this(other->_data[i], a);
      }
    } else {
      // with clipping
      Debug::error("Texture2D::lerp() with clipping not implemented");
    }
  }


  void AA_OPTIMIZE Texture2D::bubble_x(float px, float radius, Color c) {
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


  void AA_OPTIMIZE Texture2D::char_5x5(int32_t px, int32_t py, char ch,
      Color c) {
    if((px >= 0) && ((size_t)px + 5 <= _width) &&
        (py >= 0) && ((size_t)py + 5 <= _height)) {
      if((ch < 32) || (ch >= 128)) {
        return;
      }
      uint8_t const * glyph = font_5x5[ch - 32];
      for(size_t yi = 0; yi < 5; ++yi) {
        for(size_t xi = 0; xi < 5; ++xi) {
          if(glyph[yi] & (0b10000 >> xi)) {
            _data[((size_t)py + yi) * _width + ((size_t)px + xi)] = c;
          }
        }
      }
    } else {
      // with clipping
      Debug::error("Texture2D::char_5x5() with clipping not implemented");
    }
  }


  void AA_OPTIMIZE Texture2D::char_10x15(int32_t px, int32_t py, char ch,
      Color c) {
    if((px >= 0) && ((size_t)px + 10 <= _width) &&
        (py >= 0) && ((size_t)py + 15 <= _height)) {
      if(ch < '0' || ch > '9') {
        return;
      }
      uint16_t const * glyph = font_10x15_numerals[ch - '0'];
      for(size_t yi = 0; yi < 15; ++yi) {
        for(size_t xi = 0; xi < 10; ++xi) {
          if(glyph[yi] & (0b1000000000 >> xi)) {
            _data[((size_t)py + yi) * _width + ((size_t)px + xi)] = c;
          }
        }
      }
    } else {
      // with clipping
      Debug::error("Texture2D::char_10x15() with clipping not implemented");
    }
  }


  void AA_OPTIMIZE Texture2D::write_ws2811_color32(uint32_t * dest,
      size_t dest_w, size_t dest_h, bool initial_invert_x,
      size_t src_x, size_t src_y) const {
    //Debug::tracef("w: %p %2lu %2lu %c %2lu %2lu", dest, dest_w, dest_h,
    //  initial_invert_x ? 'y' : 'n', src_x, src_y);
    ptrdiff_t p_inc = initial_invert_x ? -1 : 1;
    for(size_t yi = 0; yi < dest_h; ++yi) {
      uint32_t * p = dest + ((p_inc < 0) ? (dest_w - 1) : 0) + dest_w * yi;
      for(size_t xi = 0; xi < dest_w; ++xi) {
        *p = get(src_x + xi, src_y + yi).to_ws2811_color32();
        p += p_inc;
      }
      p_inc = -p_inc;
    }
  }
}
