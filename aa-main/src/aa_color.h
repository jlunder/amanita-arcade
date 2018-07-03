#ifndef AA_COLOR_H
#define AA_COLOR_H


#include "amanita_arcade.h"


namespace aa {
  class Color {
  public:
    static Color const black;
    static Color const white;
    static Color const red;
    static Color const green;
    static Color const blue;
    static Color const pink;

    AA_OPTIMIZE Color() { }
    AA_OPTIMIZE Color(float r, float g, float b, float a = 1.0f)
      : _r(r), _g(g), _b(b), _a(a) { }
    AA_OPTIMIZE Color(Color const & c)
      : _r(c._r), _g(c._g), _b(c._b), _a(c._a) { }

    float AA_OPTIMIZE get_r() const { return _r; }
    float AA_OPTIMIZE get_g() const { return _g; }
    float AA_OPTIMIZE get_b() const { return _b; }
    float AA_OPTIMIZE get_a() const { return _a; }

    void AA_OPTIMIZE set(float r, float g, float b, float a = 1.0f) {
      _r = r; _g = g; _b = b; _a = a;
    }
    void AA_OPTIMIZE set_r(float r) { _r = r; }
    void AA_OPTIMIZE set_g(float g) { _g = g; }
    void AA_OPTIMIZE set_b(float b) { _b = b; }
    void AA_OPTIMIZE set_a(float a) { _a = a; }

    Color & AA_OPTIMIZE cie_scale_this(float l) {
      return *this *= cie(l);
    }

    Color & AA_OPTIMIZE pma_this() {
      _r *= _a;
      _g *= _a;
      _b *= _a;
      return *this;
    }

    Color & AA_OPTIMIZE lerp_this(Color const & y, float a) {
      float negA = 1.0f - a;
      _r = _r * negA + y._r * a;
      _g = _g * negA + y._g * a;
      _b = _b * negA + y._b * a;
      _a = _a * negA + y._a * a;
      return *this;
    }

    Color & AA_OPTIMIZE mix_this(Color const & y) {
      float negA = 1.0f - y._a;
      _r = _r * negA + y._r;
      _g = _g * negA + y._g;
      _b = _b * negA + y._b;
      _a = _a * negA + y._a;
      return *this;
    }

    Color & AA_OPTIMIZE elem_mul_this(Color const & y) {
      _r *= y._r;
      _g *= y._g;
      _b *= y._b;
      _a *= y._a;
      return *this;
    }

    Color AA_OPTIMIZE with_r(float r) const { return Color(r, _g, _b, _a); }
    Color AA_OPTIMIZE with_g(float g) const { return Color(_r, g, _b, _a); }
    Color AA_OPTIMIZE with_b(float b) const { return Color(_r, _g, b, _a); }
    Color AA_OPTIMIZE with_a(float a) const { return Color(_r, _g, _b, a); }

    Color AA_OPTIMIZE cie_scale(float l) const {
      return *this * cie(l);
    }

    Color AA_OPTIMIZE pma() const {
      return Color(_r * _a, _g * _a, _b * _a, _a);
    }

    Color AA_OPTIMIZE lerp(Color const & y, float a) const {
      float negA = 1.0f - a;
      return Color(
        _r * negA + y._r * a,
        _g * negA + y._g * a,
        _b * negA + y._b * a,
        _a * negA + y._a * a);
    }

    Color AA_OPTIMIZE mix(Color const & y) {
      float negA = 1.0f - y._a;
      return Color(
        _r * negA + y._r,
        _g * negA + y._g,
        _b * negA + y._b,
        _a * negA + y._a);
    }

    Color AA_OPTIMIZE elem_mul(Color const & y) const {
      return Color(_r * y._r, _g * y._g, _b * y._b, _a * y._a);
    }

    float AA_OPTIMIZE dot(Color const & y) const {
      return _r * y._r + _g * y._g + _b * y._b + _a * y._a;
    }

    Color & AA_OPTIMIZE operator *=(float a) {
      _r *= a;
      _g *= a;
      _b *= a;
      _a *= a;
      return *this;
    }

    Color & AA_OPTIMIZE operator +=(Color const & y) {
      _r += y._r;
      _g += y._g;
      _b += y._b;
      _a += y._a;
      return *this;
    }

    Color & AA_OPTIMIZE operator -=(Color const & y) {
      _r -= y._r;
      _g -= y._g;
      _b -= y._b;
      _a -= y._a;
      return *this;
    }

    uint32_t AA_OPTIMIZE to_color32() const {
      return
        (_r >= 1.0f ? 0xFF0000 : (_r >= 0.0f ?
          ((uint32_t)(_r * 256) << 16) : 0x000000)) |
        (_g >= 1.0f ? 0x00FF00 : (_g >= 0.0f ?
          ((uint32_t)(_g * 256) << 8) : 0x000000)) |
        (_b >= 1.0f ? 0x0000FF : (_b >= 0.0f ?
          ((uint32_t)(_b * 256) << 0) : 0x000000));
    }

    uint32_t AA_OPTIMIZE to_grb_color32() const {
      return
        (_g >= 1.0f ? 0xFF0000 : (_g >= 0.0f ?
          ((uint32_t)(_g * 256) << 16) : 0x000000)) |
        (_r >= 1.0f ? 0x00FF00 : (_r >= 0.0f ?
          ((uint32_t)(_r * 256) << 8) : 0x000000)) |
        (_b >= 1.0f ? 0x0000FF : (_b >= 0.0f ?
          ((uint32_t)(_b * 256) << 0) : 0x000000));
    }

    uint32_t AA_OPTIMIZE to_brg_color32() const {
      return
        (_b >= 1.0f ? 0xFF0000 : (_b >= 0.0f ?
          ((uint32_t)(_b * 256) << 16) : 0x000000)) |
        (_r >= 1.0f ? 0x00FF00 : (_r >= 0.0f ?
          ((uint32_t)(_r * 256) << 8) : 0x000000)) |
        (_g >= 1.0f ? 0x0000FF : (_g >= 0.0f ?
          ((uint32_t)(_g * 256) << 0) : 0x000000));
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

  private:
    static const size_t CIE_TABLE_SIZE = 17;
    static float cie_table[CIE_TABLE_SIZE];

    float _r, _g, _b, _a;
  };

  inline Color AA_OPTIMIZE operator +(Color const & x, Color const & y) {
    return Color(x._r + y._r, x._g + y._g, x._b + y._b, x._a + y._a);
  }

  inline Color AA_OPTIMIZE operator -(Color const & x) {
    return Color(-x._r, -x._g, -x._b, -x._a);
  }

  inline Color AA_OPTIMIZE operator -(Color const & x, Color const & y) {
    return Color(x._r - y._r, x._g - y._g, x._b - y._b, x._a - y._a);
  }

  inline Color AA_OPTIMIZE operator *(Color const & x, float y) {
    return Color(x._r * y, x._g * y, x._b * y, x._a * y);
  }

  inline Color AA_OPTIMIZE operator *(float x, Color const & y) {
    return Color(x * y._r, x * y._g, x * y._b, x * y._a);
  }

  inline Color AA_OPTIMIZE operator /(Color const & x, float y) {
    return Color(x._r / y, x._g / y, x._b / y, x._a / y);
  }

}

#endif // AA_COLOR_H
