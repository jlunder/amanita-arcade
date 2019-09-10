#include "amanita_arcade.h"

#include "aa_color.h"


namespace aa {
  Color const Color::black = Color::make(0.0f, 0.0f, 0.0f, 1.0f);
  Color const Color::white = Color::make(1.0f, 1.0f, 1.0f, 1.0f);
  Color const Color::red = Color::make(1.0f, 0.0f, 0.0f, 1.0f);
  Color const Color::green = Color::make(0.0f, 1.0f, 0.0f, 1.0f);
  Color const Color::blue = Color::make(0.0f, 0.0f, 1.0f, 1.0f);
  Color const Color::pink = Color::make(1.0f, 0.4f, 0.4f, 1.0f);
  Color const Color::transparent = Color::make(0.0f, 0.0f, 0.0f, 0.0f);

  float const cie_table[CIE_TABLE_SIZE] = {
    0.0f,
    0.00692674f,
    0.0148307f,
    0.0268838f,
    0.0441548f,
    0.0675821f,
    0.0981041f,
    0.136659f,
    0.184187f,
    0.241624f,
    0.309910f,
    0.389983f,
    0.482781f,
    0.589244f,
    0.710309f,
    0.846915f,
    1.0f,
  };


  float AA_OPTIMIZE Color::cie(float l) {
    if(l >= 1.0f) {
      return 1.0f;
    }
    else if(l >= 0.0f) {
      float il = l * (CIE_TABLE_SIZE - 1);
      size_t i = static_cast<size_t>(il);
      float a = il - floorf(il);
      float negA = 1.0f - a;
      return cie_table[i] * negA + cie_table[i + 1] * a;
    }
    else {
      return 0.0f;
    }
  }
}
