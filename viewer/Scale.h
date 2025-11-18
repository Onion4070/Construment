#pragma once
#include <cstdint>

namespace Scale {
  // Must match the order in toRP2350/Scale::Note
  enum Note : int8_t {
                                       Gs2, A2, As2, B2,
    C3, Cs3, D3, Ds3, E3, F3, Fs3, G3, Gs3, A3, As3, B3,
    C4, Cs4, D4, Ds4, E4, F4, Fs4, G4, Gs4, A4, As4, B4,
    C5, Cs5, D5, Ds5, E5, F5, Fs5, G5, Gs5, A5, As5, B5,
    C6, Cs6, D6, Ds6,
    Silence,
    COUNT
  };

  // 半音での移調ユーティリティ（オーバーフローを下限/上限にクランプ）
  inline Note transpose(Note n, int semitones) {
      int idx = (int)n + semitones;
      if (idx < 0) idx = 0;
      if (idx >= (int)COUNT) idx = (int)COUNT - 1;
      return (Note)idx;
    }
}
