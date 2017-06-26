#ifndef AA_COLOR_H

#include "amanita_arcade.h"

namespace aa {
  class Color {
  public:
    Color(float r, float g, float b, float a = 1.0f)
      : _r(r), _g(g), _b(b), _a(a) { }
    Color(Color const & c)
      : _r(c._r), _g(c._g), _b(c._b), _a(c._a) { }

    float getR() const { return _r; }
    float getG() const { return _g; }
    float getB() const { return _b; }
    float getA() const { return _a; }

    void set(float r, float g, float b, float a = 1.0f) {
      _r = r; _g = g; _b = b; _a = a;
    }
    void setR(float r) { _r = r; }
    void setG(float g) { _g = g; }
    void setB(float b) { _b = b; }
    void setA(float a) { _a = a; }

    void lerpThis(Color const & y, float a) {
      float negA = 1.0f - a;
      _r = _r * a + y._r * negA;
      _g = _g * a + y._g * negA;
      _b = _b * a + y._b * negA;
      _a = _a * a + y._a * negA;
    }

    void mixThis(Color const & y) {
      float negA = 1.0f - y._a;
      _r = _r * negA + y._r;
      _g = _g * negA + y._g;
      _b = _b * negA + y._b;
      _a = _a * negA + y._a;
    }

    Color with_r(float r) const { return Color(r, _g, _b, _a); }
    Color with_g(float g) const { return Color(_r, g, _b, _a); }
    Color with_b(float b) const { return Color(_r, _g, b, _a); }
    Color with_a(float a) const { return Color(_r, _g, _b, a); }

    Color cie_scale(float l) const {
      return *this * cie(l);
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
          ((uint32_t)(_g * 256) << 16) : 0x000000)) |
        (_r >= 1.0f ? 0x00FF00 : (_r >= 0.0f ?
          ((uint32_t)(_r * 256) << 8) : 0x000000)) |
        (_b >= 1.0f ? 0x0000FF : (_b >= 0.0f ?
          ((uint32_t)(_b * 256) << 0) : 0x000000));
    }

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
    return Color(x.getR() + y.getR(), x.getG() + y.getG(),
      x.getB() + y.getB(), x.getA() + y.getA());
  }

  inline Color operator -(Color const & x) {
    return Color(-x.getR(), -x.getG(), -x.getB(), -x.getA());
  }

  inline Color operator -(Color const & x, Color const & y) {
    return Color(x.getR() - y.getR(), x.getG() - y.getG(),
      x.getB() - y.getB(), x.getA() - y.getA());
  }

  inline Color operator *(Color const & x, float y) {
    return Color(x.getR() * y, x.getG() * y, x.getB() * y, x.getA() * y);
  }

  inline Color operator *(float x, Color const & y) {
    return Color(x * y.getR(), x * y.getG(), x * y.getB(), x * y.getA());
  }

  inline Color operator /(Color const & x, float y) {
    return Color(x.getR() / y, x.getG() / y, x.getB() / y, x.getA() / y);
  }

}

#endif // AA_COLOR_H
