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
    size_t get_stride() const { return _width; }
    Color const * get_data() const { return _data; }
    Color get(size_t x, size_t y) const { return _data[(y * _width) + x]; }
    void set(size_t x, size_t y, Color c) const {
      _data[(y * _width) + x] = c;
    }
    Color AA_OPTIMIZE sample_cn(int32_t x, int32_t y) const {
      if(x < 0) x = 0;
      if((size_t)x >= _width) x = _width - 1;
      if(y < 0) y = 0;
      if((size_t)y >= _height) y = _height - 1;
      return get((size_t)x, (size_t)y);
    }
    //Color samplef_cn(float x, float y) const;
    //Color samplef_cl(size_t x, size_t y) const;
    //Color samplef_wn(float x, float y) const;
    //Color samplef_wl(size_t x, size_t y) const;

    void fill_solid(Color c);
    void lerp_solid(Color c, float a);
    //void hline_solid(int32_t x, int32_t y, int32_t w, Color c);
    //void vline_solid(int32_t x, int32_t y, int32_t h, Color c);
    //void hline_grad(int32_t x, int32_t y, int32_t w, Color ca, Color cb);
    //void vline_grad(int32_t x, int32_t y, int32_t h, Color ca, Color cb);
    void box_solid(int32_t x, int32_t y, int32_t w, int32_t h, Color c);
    // closed gradient -- bottom right pixel is colored with exactly cx1y1
    void box_grad_c(int32_t x, int32_t y, int32_t w, int32_t h, Color cx0y0,
      Color cx1y0, Color cx0y1, Color cx1y1);
    // open gradient -- bottom right pixel is colored second to cx1y1
    void box_grad_o(int32_t x, int32_t y, int32_t w, int32_t h, Color cx0y0,
      Color cx1y0, Color cx0y1, Color cx1y1);
    void box_mask(int32_t x, int32_t y, int32_t w, int32_t h,
      Texture2D const * tex);
    void copy(Texture2D const * src);
    void mix(Texture2D const * src);
    void lerp(Texture2D const * other, float a);
    void bubble_x(float x, float r, Color c);
    void char_5x5_solid(int32_t x, int32_t y, char ch, Color c);
    void char_5x5_mask(int32_t x, int32_t y, char ch, Texture2D const * tex);
    void char_10x15_solid(int32_t x, int32_t y, char ch, Color c);
    void char_10x15_mask(int32_t x, int32_t y, char ch, Texture2D const * tex);

    void write_ws2811_color32(uint32_t * dest, size_t dest_w, size_t dest_h,
      bool initial_invert_x, size_t src_x, size_t src_y) const;

  private:
    size_t _width;
    size_t _height;
    Color * _data;
  };
}


#endif // AA_TEXTURE_2D_H
