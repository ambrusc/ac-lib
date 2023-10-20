#ifndef AC_MATH_H_
#define AC_MATH_H_

#include <math.h>
#include <stdint.h>

#define ac_max(a, b)        \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })

#define ac_min(a, b)        \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })

#define post_inc(x, dx)    \
  ({                       \
    __typeof__(x) ret = x; \
    x += dx;               \
    ret;                   \
  })

// https://en.wikipedia.org/wiki/Fast_inverse_square_root
static inline float ac_appx_inv_sqrtf(float x) {
  union {
    float f;
    uint32_t i;
  } conv = {.f = x};
  conv.i = 0x5f3759df - (conv.i >> 1);
  conv.f *= 1.5F - (x * 0.5F * conv.f * conv.f);
  return conv.f;
}

#endif  // AC_MATH_H_
