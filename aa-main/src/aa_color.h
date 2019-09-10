#ifndef AA_COLOR_H
#define AA_COLOR_H


#include "amanita_arcade.h"


namespace aa {
  static const size_t CIE_TABLE_SIZE = 17;
  extern float const cie_table[CIE_TABLE_SIZE];

#if 0
  struct Color {
    static Color const black;
    static Color const white;
    static Color const red;
    static Color const green;
    static Color const blue;
    static Color const pink;
    static Color const transparent;

    float r, g, b, a;

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
        (b >= 1.0f ? 0xFF0000 : (b >= 0.0f ?
          ((uint32_t)(b * 256) << 16) : 0x000000)) |
        (g >= 1.0f ? 0x00FF00 : (g >= 0.0f ?
          ((uint32_t)(g * 256) << 8) : 0x000000)) |
        (r >= 1.0f ? 0x0000FF : (r >= 0.0f ?
          ((uint32_t)(r * 256) << 0) : 0x000000));
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
      float white_part = std::min(std::min(r, g), b);
      float g_part = g - white_part;
      float b_part = b - white_part;
      float bal_g = g_part + white_part * 0.67f;
      float bal_b = b_part + white_part * 0.67f;
      return
        (bal_b >= 1.0f ? 0xFF0000 : (bal_b >= 0.0f ?
          ((uint32_t)(bal_b * 0.67f * 256) << 16) : 0x000000)) |
        (r >= 1.0f ? 0x00FF00 : (r >= 0.0f ?
          ((uint32_t)(r * 256) << 8) : 0x000000)) |
        (bal_g >= 1.0f ? 0x0000FF : (bal_g >= 0.0f ?
          ((uint32_t)(bal_g * 0.67f * 256) << 0) : 0x000000));
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
#else
  struct Color {
    static Color const black;
    static Color const white;
    static Color const red;
    static Color const green;
    static Color const blue;
    static Color const pink;
    static Color const transparent;

    uint32_t _c;

    static inline constexpr uint8_t AA_OPTIMIZE to_fixed(float f) {
      return (0.0f <= f) ? ((f < 1.0f) ? (uint8_t)(f * 255 + 0.5f) : 255) : 0;
    }

    static inline constexpr uint8_t AA_OPTIMIZE fixed_mul(
        uint8_t x, uint8_t y) {
      uint_fast32_t x32 = (((uint_fast32_t)x << 1) + 1);
      uint_fast32_t y32 = (((uint_fast32_t)y << 1) + 1);
      return (uint8_t)((x32 * y32 + 256) >> 10);
    }

    static constexpr uint32_t AA_OPTIMIZE fixed_mul_4_1(
        uint32_t x, uint8_t y) {
      return pack(fixed_mul(unpack_r(x), y), fixed_mul(unpack_g(x), y),
        fixed_mul(unpack_b(x), y), fixed_mul(unpack_a(x), y));
    }

    static constexpr uint32_t AA_OPTIMIZE fixed_mul_4_4(
        uint32_t x, uint32_t y) {
      return pack(fixed_mul(unpack_r(x), unpack_r(y)),
        fixed_mul(unpack_g(x), unpack_g(y)),
        fixed_mul(unpack_b(x), unpack_b(y)),
        fixed_mul(unpack_a(x), unpack_a(y)));
    }

    static inline constexpr uint8_t fixed_add_c(uint8_t x, uint8_t y) {
      uint_fast16_t sum = (uint_fast16_t)x + (uint_fast16_t)y;
      if(sum > 255) {
        return 255;
      } else {
        return (uint8_t)sum;
      }
    }

    static inline constexpr uint8_t fixed_sub_c(uint8_t x, uint8_t y) {
      uint_fast16_t sum = (uint_fast16_t)x + (uint_fast16_t)y;
      if(y > x) {
        return 0;
      }
      return (uint8_t)sum;
    }

    static inline constexpr uint32_t AA_OPTIMIZE fixed_add_4_4_c(
        uint32_t x, uint32_t y) {
      return pack(
        fixed_add_c(unpack_r(x), unpack_r(y)),
        fixed_add_c(unpack_g(x), unpack_g(y)),
        fixed_add_c(unpack_b(x), unpack_b(y)),
        fixed_add_c(unpack_a(x), unpack_a(y)));
    }

    static inline constexpr uint32_t AA_OPTIMIZE fixed_sub_4_4_c(
        uint32_t x, uint32_t y) {
      return pack(
        fixed_sub_c(unpack_r(x), unpack_r(y)),
        fixed_sub_c(unpack_g(x), unpack_g(y)),
        fixed_sub_c(unpack_b(x), unpack_b(y)),
        fixed_sub_c(unpack_a(x), unpack_a(y)));
    }

    __STATIC_FORCEINLINE uint32_t AA_OPTIMIZE fixed_add_4_4(
        uint32_t x, uint32_t y) {
      return __UQADD8(x, y);
    }

    __STATIC_FORCEINLINE uint32_t AA_OPTIMIZE fixed_sub_4_4(
        uint32_t x, uint32_t y) {
      return __UQSUB8(x, y);
    }

    __STATIC_FORCEINLINE constexpr uint32_t AA_OPTIMIZE pack(
        uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
      return ((uint32_t)r << 0) | ((uint32_t)g << 8) | ((uint32_t)b << 16)
        | ((uint32_t)a << 24);
    }

    __STATIC_FORCEINLINE constexpr uint8_t AA_OPTIMIZE unpack_r(uint32_t c) {
      return (uint8_t)((c >> 0) & 0xFF);
    }

    __STATIC_FORCEINLINE constexpr uint8_t AA_OPTIMIZE unpack_g(uint32_t c) {
      return (uint8_t)((c >> 8) & 0xFF);
    }

    __STATIC_FORCEINLINE constexpr uint8_t AA_OPTIMIZE unpack_b(uint32_t c) {
      return (uint8_t)((c >> 16) & 0xFF);
    }

    __STATIC_FORCEINLINE constexpr uint8_t AA_OPTIMIZE unpack_a(uint32_t c) {
      return (uint8_t)((c >> 24) & 0xFF);
    }

    __STATIC_FORCEINLINE constexpr Color AA_OPTIMIZE make(float n_r, float n_g,
        float n_b, float n_a = 1.0f) {
      Color c = {
        pack(to_fixed(n_r), to_fixed(n_g), to_fixed(n_b), to_fixed(n_a))
      };
      return c;
    }

    __STATIC_FORCEINLINE constexpr Color AA_OPTIMIZE fixed_make(
        uint8_t n_r, uint8_t n_g, uint8_t n_b, uint8_t n_a = 255) {
      Color c = { pack(n_r, n_g, n_b, n_a) };
      return c;
    }

    __STATIC_FORCEINLINE constexpr Color AA_OPTIMIZE packed_make(uint32_t n_c) {
      Color c = { n_c };
      return c;
    }

    void AA_OPTIMIZE set(float n_r, float n_g, float n_b, float n_a = 1.0f) {
      _c = pack(to_fixed(n_r), to_fixed(n_g), to_fixed(n_b), to_fixed(n_a));
    }

    Color & AA_OPTIMIZE cie_scale_this(float l) {
      *this = cie_scale(l);
      return *this;
    }

    Color & AA_OPTIMIZE pma_this() {
      *this = pma();
      return *this;
    }

    Color & AA_OPTIMIZE lerp_this(Color const & y, float f_a) {
      *this = lerp(y, f_a);
      return *this;
    }

    Color & AA_OPTIMIZE mix_this(Color const & y) {
      *this = mix(y);
      return *this;
    }

    inline constexpr Color AA_OPTIMIZE with_r(float n_r) const {
      return packed_make(
        (_c & ~(0xFFLU << 0)) | ((uint32_t)to_fixed(n_r) << 0));
    }
    inline constexpr Color AA_OPTIMIZE with_g(float n_g) const {
      return packed_make(
        (_c & ~(0xFFLU << 8)) | ((uint32_t)to_fixed(n_g) << 8));
    }
    inline constexpr Color AA_OPTIMIZE with_b(float n_b) const {
      return packed_make(
        (_c & ~(0xFFLU << 16)) | ((uint32_t)to_fixed(n_b) << 16));
    }
    inline constexpr Color AA_OPTIMIZE with_a(float n_a) const {
      return packed_make(
        (_c & ~(0xFFLU << 24)) | ((uint32_t)to_fixed(n_a) << 24));
    }

    inline Color AA_OPTIMIZE cie_scale(float l) const {
      return packed_make(fixed_mul_4_1(_c, to_fixed(cie(l))));
    }

    inline constexpr Color AA_OPTIMIZE pma() const {
      uint8_t a = unpack_a(_c);
      return fixed_make(fixed_mul(unpack_r(_c), a),
        fixed_mul(unpack_g(_c), a), fixed_mul(unpack_b(_c), a), a);
    }

    inline Color AA_OPTIMIZE lerp(
        Color const & y, float f_a) const {
      uint8_t a = to_fixed(f_a);
      uint8_t neg_a = 255 - a;
      return packed_make(
        fixed_add_4_4(fixed_mul_4_1(_c, neg_a), fixed_mul_4_1(y._c, a)));
    }

    inline Color AA_OPTIMIZE mix(Color const & y) {
      uint8_t neg_a = 255 - unpack_a(y._c);
      return packed_make(fixed_add_4_4(fixed_mul_4_1(_c, neg_a), y._c));
    }

    Color & AA_OPTIMIZE operator *=(float y) {
      _c = fixed_mul_4_1(_c, to_fixed(y));
      return *this;
    }

    Color & AA_OPTIMIZE operator +=(Color const & y) {
      _c = fixed_add_4_4(_c, y._c);
      return *this;
    }

    Color & AA_OPTIMIZE operator -=(Color const & y) {
      _c = fixed_sub_4_4(_c, y._c);
      return *this;
    }

    inline constexpr uint32_t AA_OPTIMIZE to_color32() const {
      return _c;
    }

    inline constexpr uint32_t AA_OPTIMIZE to_grb_color32() const {
      return ((uint32_t)unpack_b(_c) << 0) | ((uint32_t)unpack_r(_c) << 8)
        | ((uint32_t)unpack_g(_c) << 16);
    }

    inline constexpr uint32_t AA_OPTIMIZE to_brg_color32() const {
      uint_fast8_t r = unpack_r(_c);
      uint_fast8_t g = unpack_g(_c);
      uint_fast8_t b = unpack_b(_c);
      uint_fast8_t white_part = std::min(r, std::min(g, b));
      uint_fast8_t white_adjust = (white_part >> 2) - (white_part >> 4);
      uint_fast8_t bal_g = g - white_adjust;
      uint_fast8_t bal_b = b - white_adjust;
      return (((uint32_t)bal_g) << 0) | (((uint32_t)r) << 8)
        | (((uint32_t)bal_b) << 16);
    }

    // Takes a normalized float on [0..1] representing apparent brightness and
    // returns a float on [0..1] with the appropriate linear output power to
    // achieve that apparent brightness.
    static float cie(float l);

    friend Color operator +(Color const & x, Color const & y);
    friend Color operator -(Color const & x, Color const & y);
    friend constexpr Color operator *(Color const & x, float y);
  };

  inline Color AA_OPTIMIZE operator +(
      Color const & x, Color const & y) {
    return Color::packed_make(Color::fixed_add_4_4(x._c, y._c));
  }

  inline Color AA_OPTIMIZE operator -(
      Color const & x, Color const & y) {
    return Color::packed_make(Color::fixed_sub_4_4(x._c, y._c));
  }

  inline constexpr Color AA_OPTIMIZE operator *(Color const & x, float y) {
    return Color::packed_make(Color::fixed_mul_4_1(x._c, Color::to_fixed(y)));
  }
#endif

}

#endif // AA_COLOR_H
