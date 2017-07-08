#include "amanita_arcade.h"

#include "aa_texture_2d.h"


namespace aa {
  namespace {
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
        0b00000,
        0b01010,
        0b11111,
        0b01010,
        0b00000,
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
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
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
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '='
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '>'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '?'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '@'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'A'
        0b00100,
        0b01010,
        0b11111,
        0b10001,
        0b10001,
      },
      { // 'B'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'C'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'D'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'E'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'F'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'G'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'H'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'I'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'J'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'K'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'L'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'M'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'N'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'O'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'P'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'Q'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'R'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'S'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'T'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'U'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'V'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'W'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'X'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'Y'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'Z'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '['
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '\'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // ']'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '^'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '_'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '`'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'a'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'b'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'c'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'd'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'e'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'f'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'g'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'h'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'i'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'j'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'k'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'l'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'm'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'n'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'o'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'p'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'q'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'r'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 's'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 't'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'u'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'v'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'w'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'x'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'y'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // 'z'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '{'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '|'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '}'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
      { // '~'
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
      },
    };
  }


  void Texture2D::fill_solid(Color c) {
    size_t count = _width * _height;
    for(size_t i = 0; i < count; ++i) {
      _data[i] = c;
    }
  }


  void Texture2D::lerp_solid(Color c, float a) {
    size_t count = _width * _height;
    for(size_t i = 0; i < count; ++i) {
      _data[i].lerp_this(c, a);
    }
  }


  void Texture2D::copy(Texture2D const * src) {
    if((_width == src->_width) && (_height == src->_height)) {
      size_t count = _width * _height;
      memcpy(_data, src->_data, count * sizeof (Color));
    } else {
      // with clipping
      Debug::error("Texture2D::copy() with clipping not implemented");
    }
  }


  void Texture2D::mix(Texture2D const * src) {
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


  void Texture2D::lerp(Texture2D const * other, float a) {
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


  void Texture2D::bubble_x(float px, float radius, Color c) {
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


  void Texture2D::char_5x5(int32_t px, int32_t py, char ch, Color c) {

  }


  void Texture2D::write_ws2811_color32(uint32_t * dest, size_t dest_width,
      size_t dest_height, int32_t src_x, int32_t src_y) {

  }
}
