#ifndef AA_TEXTURE_2D_H
#define AA_TEXTURE_2D_H


#include "amanita_arcade.h"

#include "aa_color.h"


namespace aa {
  class Texture2D {
  public:
    Texture2D() : _width(), _height(), _data() { }

    void init(size_t width, size_t height, Color * data) {
      _width = width;
      _height = height;
      _data = data;
    }

    size_t get_width() const { return _width; }
    size_t get_height() const { return _height; }
    Color sample(size_t x, size_t y) const { return _data[(y * _width) + x]; }

    void fill_solid(Color c);
    void copy(Texture2D const * src);
    void mix(Texture2D const * src);
    void lerp(Texture2D const * other, float a);
    void bubble_x(float x, float r, Color c);
    void char_5x5(int32_t x, int32_t y, char ch, Color c);

    void write_ws2811_color32(uint32_t * dest, size_t dest_width,
      size_t dest_height, int32_t src_x, int32_t src_y);

  private:
    size_t _width;
    size_t _height;
    Color * _data;
  };
}


#endif // AA_TEXTURE_2D_H
