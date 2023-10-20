#ifndef AC_STR_H_
#define AC_STR_H_

#include <limits.h>
#include <stdio.h>

#include "ac_alloc.h"

// Extensible string type.
typedef ac_lista_char ac_str;

// Initialize an empty string with the given allocator (doesn't allocate).
static inline ac_str ac_str_init(ac_allocator2 alloc);

// Frees the string using its allocator and clears all fields.
static inline void ac_str_free(ac_str* s);

// Maximum number of characters that can be printed.
#if defined RSIZE_MAX
enum : size_t { AC_PRINT_MAX = RSIZE_MAX };
#else
enum : size_t { AC_PRINT_MAX = INT_MAX };
#endif

static inline int ac_print(ac_str s);
static inline int ac_fprint(FILE* f, ac_str s);

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

static inline ac_str ac_str_init(ac_allocator2 alloc) {
  return (ac_str){alloc, NULL, 0, 0};
}

static inline void ac_str_free(ac_str* s) {
  ac_free2(s->alloc, ac_lista_mem(s));
  *s = (ac_str){};
}

// Formatted string printing to an extensible buffer (using snprintf).
// Returns the number of characters printed, omitting the null terminator.
// If the buffer is too small and allocation fails, returns 0.
// NOTE: null-terminator is omitted from the length but always written.
//      ac_to_str(str*,    c-format-string, var-args...)
#define ac_to_str(str_ptr, ...)                                                \
  ({                                                                           \
    int printed = 0;                                                           \
    do {                                                                       \
      ac_str* sss_ = (str_ptr);                                                \
                                                                               \
      if (sss_->cap > sss_->len) {                                             \
        const size_t size_raw = sss_->cap - sss_->len;                         \
        const size_t size = size_raw < AC_PRINT_MAX ? size_raw : AC_PRINT_MAX; \
        printed = snprintf(sss_->data + sss_->len, size, __VA_ARGS__);         \
        if (printed >= 0 && (size_t)printed < size) {                          \
          sss_->len += printed;                                                \
          break;                                                               \
        }                                                                      \
      } else {                                                                 \
        printed = snprintf(NULL, 0, __VA_ARGS__);                              \
      }                                                                        \
                                                                               \
      const size_t fit_cap = sss_->cap + printed + 1;                          \
      const size_t grow_cap = 2 * sss_->cap;                                   \
      const size_t cap = fit_cap > grow_cap ? fit_cap : grow_cap;              \
                                                                               \
      ac_lista_realloc(sss_, cap);                                             \
                                                                               \
      if (sss_->cap > sss_->len) {                                             \
        const size_t size = sss_->cap - sss_->len;                             \
        printed = snprintf(sss_->data + sss_->len, size, __VA_ARGS__);         \
        if (printed >= 0 && (size_t)printed < size) {                          \
          sss_->len += printed;                                                \
        } else {                                                               \
          printed = 0;                                                         \
        }                                                                      \
      }                                                                        \
    } while (false);                                                           \
    printed;                                                                   \
  })

// Repeats the given character 'n' times and appends it to the string.
static inline void ac_str_repeat_char(ac_str* s, char c, size_t count) {
  const size_t len_after_write = s->len + count;
  if (len_after_write >= s->cap) ac_lista_realloc(s, len_after_write);
  memset(s->data + s->len, c, count);
  s->len = len_after_write;
}

// Prints the string, never exceeding size whether or not it's
// null-terminated.
static inline int ac_fprint(FILE* f, ac_str s) {
  return fwrite(s.data, /*size=*/1, s.len, f);
}

// Prints the string, never exceeding size whether or not it's
// null-terminated.
static inline int ac_print(ac_str s) { return ac_fprint(stdout, s); }

//------------------------------------------------------------------------------
// Define string printer types
//------------------------------------------------------------------------------

#define ac_to_str_(type) ac_to_str_##type

#define ac_to_str_define_type(fmt, type)            \
  static inline void ac_to_str_(type)(ac_str* str, const void* a) { \
    ac_to_str(str, fmt, *(const type*)a);           \
  }
ac_to_str_define_type("%c", char);
ac_to_str_define_type("%" PRIi8, int8_t);
ac_to_str_define_type("%" PRIi16, int16_t);
ac_to_str_define_type("%" PRIi32, int32_t);
ac_to_str_define_type("%" PRIi64, int64_t);
ac_to_str_define_type("%" PRIu8, uint8_t);
ac_to_str_define_type("%" PRIu16, uint16_t);
ac_to_str_define_type("%" PRIu32, uint32_t);
ac_to_str_define_type("%" PRIu64, uint64_t);
ac_to_str_define_type("%f", float);
ac_to_str_define_type("%f", double) ;

static inline void ac_to_str_cstr(ac_str* str, const char* in) {  \
  ac_to_str(str, "%s", in);
}

#endif  // AC_STR_H_
