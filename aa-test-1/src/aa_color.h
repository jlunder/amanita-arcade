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
      _r = _r * a + y.getR() * negA;
      _g = _g * a + y.getG() * negA;
      _b = _b * a + y.getB() * negA;
      _a = _a * a + y.getA() * negA;
    }

    void mixThis(Color const & y) {
      float negA = 1.0f - y.getA();
      _r = _r * negA + y.getR();
      _g = _g * negA + y.getG();
      _b = _b * negA + y.getB();
      _a = _a * negA + y.getA();
    }

    Color withR(float r) const { return Color(r, _g, _b, _a); }
    Color withG(float g) const { return Color(_r, g, _b, _a); }
    Color withB(float b) const { return Color(_r, _g, b, _a); }
    Color withA(float a) const { return Color(_r, _g, _b, a); }

    static Color lerp(Color const & x, Color const & y, float a) {
      float negA = 1.0f - a;
      return Color(x.getR() * a + y.getR() * negA,
        x.getG() * a + y.getG() * negA,
        x.getB() * a + y.getB() * negA,
        x.getA() * a + y.getA() * negA);
    }

    static Color mix(Color const & x, Color const & y) {
      float negA = 1.0f - y.getA();
      return Color(
        x.getR() * negA + y.getR(),
        x.getG() * negA + y.getG(),
        x.getB() * negA + y.getB(),
        x.getA() * negA + y.getA());
    }

    static inline Color elemMul(Color const & x, Color const & y) {
      return Color(x.getR() * y.getR(), x.getG() * y.getG(),
        x.getB() * y.getB(), x.getA() * y.getA());
    }

    static inline float dot(Color const & x, Color const & y) {
      return x.getR() * y.getR() + x.getG() * y.getG() +
        x.getB() * y.getB() + x.getA() * y.getA();
    }

    Color & operator *=(float a) {
      _r *= a;
      _g *= a;
      _b *= a;
      _a *= a;
      return *this;
    }

    Color & operator +=(Color const & y) {
      _r += y.getR();
      _g += y.getG();
      _b += y.getB();
      _a += y.getA();
      return *this;
    }

    Color & operator -=(Color const & y) {
      _r -= y.getR();
      _g -= y.getG();
      _b -= y.getB();
      _a -= y.getA();
      return *this;
    }

  private:
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
