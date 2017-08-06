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

    Color() { }
    Color(float r, float g, float b, float a = 1.0f)
      : _r(r), _g(g), _b(b), _a(a) { }
    Color(Color const & c)
      : _r(c._r), _g(c._g), _b(c._b), _a(c._a) { }

    float get_r() const { return _r; }
    float get_g() const { return _g; }
    float get_b() const { return _b; }
    float get_a() const { return _a; }

    void set(float r, float g, float b, float a = 1.0f) {
      _r = r; _g = g; _b = b; _a = a;
    }
    void set_r(float r) { _r = r; }
    void set_g(float g) { _g = g; }
    void set_b(float b) { _b = b; }
    void set_a(float a) { _a = a; }

    Color & cie_scale_this(float l) {
      return *this *= cie(l);
    }

    Color & pma_this() {
      _r *= _a;
      _g *= _a;
      _b *= _a;
      return *this;
    }

    Color & lerp_this(Color const & y, float a) {
      float negA = 1.0f - a;
      _r = _r * negA + y._r * a;
      _g = _g * negA + y._g * a;
      _b = _b * negA + y._b * a;
      _a = _a * negA + y._a * a;
      return *this;
    }

    Color & mix_this(Color const & y) {
      float negA = 1.0f - y._a;
      _r = _r * negA + y._r;
      _g = _g * negA + y._g;
      _b = _b * negA + y._b;
      _a = _a * negA + y._a;
      return *this;
    }

    Color & elem_mul_this(Color const & y) {
      _r *= y._r;
      _g *= y._g;
      _b *= y._b;
      _a *= y._a;
      return *this;
    }

    Color with_r(float r) const { return Color(r, _g, _b, _a); }
    Color with_g(float g) const { return Color(_r, g, _b, _a); }
    Color with_b(float b) const { return Color(_r, _g, b, _a); }
    Color with_a(float a) const { return Color(_r, _g, _b, a); }

    Color cie_scale(float l) const {
      return *this * cie(l);
    }

    Color pma() const {
      return Color(_r * _a, _g * _a, _b * _a, _a);
    }

    Color lerp(Color const & y, float a) const {
      float negA = 1.0f - a;
      return Color(
        _r * negA + y._r * a,
        _g * negA + y._g * a,
        _b * negA + y._b * a,
        _a * negA + y._a * a);
    }

    Color mix(Color const & y) {
      float negA = 1.0f - y._a;
      return Color(
        _r * negA + y._r,
        _g * negA + y._g,
        _b * negA + y._b,
        _a * negA + y._a);
    }

    Color elem_mul(Color const & y) const {
      return Color(_r * y._r, _g * y._g, _b * y._b, _a * y._a);
    }

    float dot(Color const & y) const {
      return _r * y._r + _g * y._g + _b * y._b + _a * y._a;
    }

    Color & operator *=(float a) {
      _r *= a;
      _g *= a;
      _b *= a;
      _a *= a;
      return *this;
    }

    Color & operator +=(Color const & y) {
      _r += y._r;
      _g += y._g;
      _b += y._b;
      _a += y._a;
      return *this;
    }

    Color & operator -=(Color const & y) {
      _r -= y._r;
      _g -= y._g;
      _b -= y._b;
      _a -= y._a;
      return *this;
    }

    uint32_t to_color32() const {
      return
        (_r >= 1.0f ? 0xFF0000 : (_r >= 0.0f ?
          ((uint32_t)(_r * 256) << 16) : 0x000000)) |
        (_g >= 1.0f ? 0x00FF00 : (_g >= 0.0f ?
          ((uint32_t)(_g * 256) << 8) : 0x000000)) |
        (_b >= 1.0f ? 0x0000FF : (_b >= 0.0f ?
          ((uint32_t)(_b * 256) << 0) : 0x000000));
    }

    uint32_t to_ws2811_color32() const {
      return
        (_g >= 1.0f ? 0xFF0000 : (_g >= 0.0f ?
          ((uint32_t)(_b * 256) << 16) : 0x000000)) |
        (_r >= 1.0f ? 0x00FF00 : (_r >= 0.0f ?
          ((uint32_t)(_r * 256) << 8) : 0x000000)) |
        (_b >= 1.0f ? 0x0000FF : (_b >= 0.0f ?
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

  inline Color operator +(Color const & x, Color const & y) {
    return Color(x._r + y._r, x._g + y._g, x._b + y._b, x._a + y._a);
  }

  inline Color operator -(Color const & x) {
    return Color(-x._r, -x._g, -x._b, -x._a);
  }

  inline Color operator -(Color const & x, Color const & y) {
    return Color(x._r - y._r, x._g - y._g, x._b - y._b, x._a - y._a);
  }

  inline Color operator *(Color const & x, float y) {
    return Color(x._r * y, x._g * y, x._b * y, x._a * y);
  }

  inline Color operator *(float x, Color const & y) {
    return Color(x * y._r, x * y._g, x * y._b, x * y._a);
  }

  inline Color operator /(Color const & x, float y) {
    return Color(x._r / y, x._g / y, x._b / y, x._a / y);
  }

}

#endif // AA_COLOR_H
