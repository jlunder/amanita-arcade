#ifndef AA_COLOR_H
#define AA_COLOR_H


#include "amanita_arcade.h"


namespace aa {
  static const size_t CIE_TABLE_SIZE = 17;
  extern float const cie_table[CIE_TABLE_SIZE];

  struct Color {
    float r, g, b, a;

    static Color const black;
    static Color const white;
    static Color const red;
    static Color const green;
    static Color const blue;
    static Color const pink;
    static Color const transparent;

    static Color AA_OPTIMIZE make(float n_r, float n_g, float n_b,
        float n_a = 1.0f) {
      Color c = { n_r, n_g, n_b, n_a };
      return c;
    }

    void AA_OPTIMIZE set(float n_r, float n_g, float n_b, float n_a = 1.0f) {
      r = n_r; g = n_g; b = n_b; a = n_a;
    }

    Color & AA_OPTIMIZE cie_scale_this(float l) {
      return *this *= cie(l);
    }

    Color & AA_OPTIMIZE pma_this() {
      r *= a;
      g *= a;
      b *= a;
      return *this;
    }

    Color & AA_OPTIMIZE lerp_this(Color const & y, float n_a) {
      float neg_a = 1.0f - n_a;
      r = r * neg_a + y.r * n_a;
      g = g * neg_a + y.g * n_a;
      b = b * neg_a + y.b * n_a;
      a = a * neg_a + y.a * n_a;
      return *this;
    }

    Color & AA_OPTIMIZE mix_this(Color const & y) {
      float neg_a = 1.0f - y.a;
      r = r * neg_a + y.r;
      g = g * neg_a + y.g;
      b = b * neg_a + y.b;
      a = a * neg_a + y.a;
      return *this;
    }

    Color & AA_OPTIMIZE elem_mul_this(Color const & y) {
      r *= y.r;
      g *= y.g;
      b *= y.b;
      a *= y.a;
      return *this;
    }

    Color AA_OPTIMIZE with_r(float n_r) const {
      return Color::make(n_r, g, b, a);
    }
    Color AA_OPTIMIZE with_g(float n_g) const {
      return Color::make(r, n_g, b, a);
    }
    Color AA_OPTIMIZE with_b(float n_b) const {
      return Color::make(r, g, n_b, a);
    }
    Color AA_OPTIMIZE with_a(float n_a) const {
      return Color::make(r, g, b, n_a);
    }

    Color AA_OPTIMIZE cie_scale(float l) const {
      return *this * cie(l);
    }

    Color AA_OPTIMIZE pma() const {
      return Color::make(r * a, g * a, b * a, a);
    }

    Color AA_OPTIMIZE lerp(Color const & y, float n_a) const {
      float neg_a = 1.0f - n_a;
      return Color::make(
        r * neg_a + y.r * n_a,
        g * neg_a + y.g * n_a,
        b * neg_a + y.b * n_a,
        a * neg_a + y.a * n_a);
    }

    Color AA_OPTIMIZE mix(Color const & y) {
      float neg_a = 1.0f - y.a;
      return Color::make(
        r * neg_a + y.r,
        g * neg_a + y.g,
        b * neg_a + y.b,
        a * neg_a + y.a);
    }

    Color AA_OPTIMIZE elem_mul(Color const & y) const {
      return Color::make(r * y.r, g * y.g, b * y.b, a * y.a);
    }

    float AA_OPTIMIZE dot(Color const & y) const {
      return r * y.r + g * y.g + b * y.b + a * y.a;
    }

    Color & AA_OPTIMIZE operator *=(float n_a) {
      r *= n_a;
      g *= n_a;
      b *= n_a;
      a *= n_a;
      return *this;
    }

    Color & AA_OPTIMIZE operator +=(Color const & y) {
      r += y.r;
      g += y.g;
      b += y.b;
      a += y.a;
      return *this;
    }

    Color & AA_OPTIMIZE operator -=(Color const & y) {
      r -= y.r;
      g -= y.g;
      b -= y.b;
      a -= y.a;
      return *this;
    }

    uint32_t AA_OPTIMIZE to_color32() const {
      return
        (r >= 1.0f ? 0xFF0000 : (r >= 0.0f ?
          ((uint32_t)(r * 256) << 16) : 0x000000)) |
        (g >= 1.0f ? 0x00FF00 : (g >= 0.0f ?
          ((uint32_t)(g * 256) << 8) : 0x000000)) |
        (b >= 1.0f ? 0x0000FF : (b >= 0.0f ?
          ((uint32_t)(b * 256) << 0) : 0x000000));
    }

    uint32_t AA_OPTIMIZE to_grb_color32() const {
      return
        (g >= 1.0f ? 0xFF0000 : (g >= 0.0f ?
          ((uint32_t)(g * 256) << 16) : 0x000000)) |
        (r >= 1.0f ? 0x00FF00 : (r >= 0.0f ?
          ((uint32_t)(r * 256) << 8) : 0x000000)) |
        (b >= 1.0f ? 0x0000FF : (b >= 0.0f ?
          ((uint32_t)(b * 256) << 0) : 0x000000));
    }

    uint32_t AA_OPTIMIZE to_brg_color32() const {
      return
        (b >= 1.0f ? 0xFF0000 : (b >= 0.0f ?
          ((uint32_t)(b * 256) << 16) : 0x000000)) |
        (r >= 1.0f ? 0x00FF00 : (r >= 0.0f ?
          ((uint32_t)(r * 256) << 8) : 0x000000)) |
        (g >= 1.0f ? 0x0000FF : (g >= 0.0f ?
          ((uint32_t)(g * 256) << 0) : 0x000000));
    }

    // Takes a normalized float on [0..1] representing apparent brightness and
    // returns a float on [0..1] with the appropriate linear output power to
    // achieve that apparent brightness.
    static float cie(float l);

    friend Color operator +(Color const & x, Color const & y);
    friend Color operator -(Color const & x);
    friend Color operator -(Color const & x, Color const & y);
    friend Color operator *(Color const & x, float y);
    friend Color operator *(float x, Color const & y);
    friend Color operator /(Color const & x, float y);
  };

  inline Color AA_OPTIMIZE operator +(Color const & x, Color const & y) {
    return Color::make(x.r + y.r, x.g + y.g, x.b + y.b, x.a + y.a);
  }

  inline Color AA_OPTIMIZE operator -(Color const & x) {
    return Color::make(-x.r, -x.g, -x.b, -x.a);
  }

  inline Color AA_OPTIMIZE operator -(Color const & x, Color const & y) {
    return Color::make(x.r - y.r, x.g - y.g, x.b - y.b, x.a - y.a);
  }

  inline Color AA_OPTIMIZE operator *(Color const & x, float y) {
    return Color::make(x.r * y, x.g * y, x.b * y, x.a * y);
  }

  inline Color AA_OPTIMIZE operator *(float x, Color const & y) {
    return Color::make(x * y.r, x * y.g, x * y.b, x * y.a);
  }

  inline Color AA_OPTIMIZE operator /(Color const & x, float y) {
    return Color::make(x.r / y, x.g / y, x.b / y, x.a / y);
  }

}

#endif // AA_COLOR_H
