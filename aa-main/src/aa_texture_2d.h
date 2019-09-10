#ifndef AA_TEXTURE_2D_H
#define AA_TEXTURE_2D_H


#include "amanita_arcade.h"

#include "aa_color.h"


namespace aa {
  class Texture2D {
  public:
    struct OpenGradient {
      Color cx0y0;
      Color cx1y0;
      Color cx0y1;
      Color cx1y1;
    };
    struct ClosedGradient {
      Color cx0y0;
      Color cx1y0;
      Color cx0y1;
      Color cx1y1;
    };

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
    void set(size_t x, size_t y, Color const & c) const {
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

    //void hline_set(int32_t x, int32_t y, int32_t w, Color const & c);
    //void vline_set(int32_t x, int32_t y, int32_t h, Color const & c);
    //void hline_grad(int32_t x, int32_t y, int32_t w, Color const & ca,
    //  Color const & cb);
    //void vline_grad(int32_t x, int32_t y, int32_t h, Color const & ca,
    //  Color const & cb);
    void box_set(int32_t x, int32_t y, int32_t w, int32_t h,
      Color const & c);
    // closed gradient -- bottom right pixel is colored with exactly cx1y1
    void box_set(int32_t x, int32_t y, int32_t w, int32_t h,
      ClosedGradient const & grad);
    // open gradient -- bottom right pixel is colored second to cx1y1
    void box_set(int32_t x, int32_t y, int32_t w, int32_t h,
      OpenGradient const & grad);
    void box_set(int32_t x, int32_t y, int32_t w, int32_t h,
      Texture2D const * tex);
    void box_set(int32_t x, int32_t y, int32_t w, int32_t h,
      Texture2D const * tex, size_t x_ofs, size_t y_ofs);
    void box_lerp(int32_t x, int32_t y, int32_t w, int32_t h,
      Color const & c, float a);
    void fill_set(Color const & c);
    void fill_set(Texture2D const * src) { fill_set(src, 0, 0); }
    void fill_set(Texture2D const * src, size_t x_ofs, size_t y_ofs);
    void fill_mix(Color const & c);
    void fill_mix(Texture2D const * src) { fill_mix(src, 0, 0); }
    void fill_mix(Texture2D const * src, size_t x_ofs, size_t y_ofs);
    void fill_lerp(Color const & c, float a);
    void fill_lerp(Texture2D const * src, float a) {
      fill_lerp(src, 0, 0, a);
    }
    void fill_lerp(Texture2D const * src, size_t x_ofs, size_t y_ofs, float a);
    void fill_bubble_x(float x, float r, Color const & c);
    void char_5x5_set(int32_t x, int32_t y, char ch, bool invert, Color const & c);
    void char_5x5_set(int32_t x, int32_t y, char const * str, Color const & c);
    void char_5x5_set(int32_t x, int32_t y, char ch, bool invert,
      Texture2D const * tex);
    void char_5x5_set(int32_t x, int32_t y, char const * str,
      Texture2D const * tex);
    void char_10x15_set(int32_t x, int32_t y, char ch, bool invert, Color const & c);
    void char_10x15_set(int32_t x, int32_t y, char const * str, Color const & c);
    void char_10x15_set(int32_t x, int32_t y, char ch, bool invert,
      Texture2D const * tex);
    void char_10x15_set(int32_t x, int32_t y, char const * str,
      Texture2D const * tex);

    void write_grb_color32(uint32_t * dest, size_t dest_w, size_t dest_h,
      bool initial_invert_x, size_t src_x, size_t src_y) const;
    void write_brg_color32(uint32_t * dest, size_t dest_w, size_t dest_h,
      bool initial_invert_x, size_t src_x, size_t src_y) const;

  private:
    size_t _width;
    size_t _height;
    Color * _data;
  };


  template<size_t W, size_t H>
  class AutoTexture2D : public Texture2D
  {
  public:
    AutoTexture2D() {
      init(W, H, _data_alloc);
    }

  private:
    Color _data_alloc[W * H];
  };
}


#endif // AA_TEXTURE_2D_H
