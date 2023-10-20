#ifndef AC_ALLOC_H_
#define AC_ALLOC_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//------------------------------------------------------------------------------
// Basic Memory Block.
//------------------------------------------------------------------------------

typedef struct ac_mem {
  void* data;
  size_t cap;
} ac_mem;

//------------------------------------------------------------------------------
// Basic Allocation.
//------------------------------------------------------------------------------

typedef ac_mem (*ac_alloc_fn2)(void* state, size_t cap);
typedef void (*ac_free_fn2)(void* state, ac_mem);

typedef struct ac_allocator2 {
  void* state;
  ac_alloc_fn2 alloc;
  ac_free_fn2 free;
} ac_allocator2;

static inline bool ac_allocator2_is_empty(ac_allocator2 a) {
  return (!a.state && !a.alloc && !a.free);
}

static inline ac_mem ac_alloc2(ac_allocator2 a, size_t cap) {
  if (a.alloc) return a.alloc(a.state, cap);
  return (ac_mem){};
}

static inline void ac_free2(ac_allocator2 a, ac_mem m) {
  if (a.free) a.free(a.state, m);
}

//------------------------------------------------------------------------------
// Mallocation.
//------------------------------------------------------------------------------

static inline ac_mem ac_sys_malloc(void* state, size_t cap) {
  (void)state;
  return (ac_mem){malloc(cap), cap};
}

static inline void ac_sys_free(void* state, ac_mem m) {
  (void)state;
  if (m.data) free(m.data);
}

static inline ac_allocator2 ac_mallocator2() {
  return (ac_allocator2){
      .alloc = &ac_sys_malloc,
      .free = &ac_sys_free,
  };
}

//------------------------------------------------------------------------------
// Contiguous List With an Allocator.
//------------------------------------------------------------------------------

#define ac_lista(element_type) ac_lista_##element_type

#define ac_lista_define_type(element_type) \
  typedef struct ac_lista(element_type) {  \
    ac_allocator2 alloc;                   \
    element_type* data;                    \
    size_t len;                            \
    size_t cap;                            \
  }                                        \
  ac_lista(element_type)

// Extract a 'mem' struct from a lista.
#define ac_lista_mem(lista_ptr) \
  (ac_mem) { (lista_ptr)->data, (lista_ptr)->cap }

// Free the list memory. Nop if null.
#define ac_lista_free(lista_ptr) \
  ac_free2((lista_ptr)->alloc, ac_lista_mem(lista_ptr))

// Reallocate a lista, copying the range [0, len) to the new memory.
// If the 'new_cap' is smaller than 'len', only [0, new_cap) items are retained.
// If the list has no 'data' and no 'alloc', the mallocator is assigned.
// However, if the list has 'data' but no 'alloc', this operation does nothing.
#define ac_lista_realloc(lista_ptr, new_cap)                                \
  do {                                                                      \
    if (ac_allocator2_is_empty((lista_ptr)->alloc) && !(lista_ptr)->data) { \
      (lista_ptr)->alloc = ac_mallocator2();                                \
    }                                                                       \
                                                                            \
    ac_allocator2 alloc = (lista_ptr)->alloc;                               \
    const size_t element_size = sizeof(__typeof__(*(lista_ptr)->data));     \
    void* const new_data = ac_alloc2(alloc, element_size * new_cap).data;   \
                                                                            \
    if (new_data) {                                                         \
      const size_t len = (lista_ptr)->len;                                  \
      const size_t new_len = len < new_cap ? len : new_cap;                 \
      const size_t copy_size = element_size * new_len;                      \
      if (new_data && (lista_ptr)->data && copy_size) {                     \
        memcpy(new_data, (lista_ptr)->data, copy_size);                     \
      }                                                                     \
      ac_free2(alloc, ac_lista_mem(lista_ptr));                             \
      (lista_ptr)->data = new_data;                                         \
      (lista_ptr)->len = new_len;                                           \
      (lista_ptr)->cap = new_cap;                                           \
    }                                                                       \
  } while (false)

// Pushes an item onto the lista, expanding the memory if necessary.
// Returns a pointer to the new item, or NULL if allocation failed.
// The new capacity = max(min_cap, cap * m + b).
#define ac_lista_next_exg(lista_ptr, m, b, min_cap)               \
  ({                                                              \
    if ((lista_ptr)->len >= (lista_ptr)->cap) {                   \
      const size_t new_cap_raw = (lista_ptr)->cap * m + b;        \
      const size_t new_cap = new_cap_raw ? new_cap_raw : min_cap; \
      ac_lista_realloc(lista_ptr, new_cap);                       \
    }                                                             \
    __typeof__((lista_ptr)->data) next;                           \
    if ((lista_ptr)->len < (lista_ptr)->cap) {                    \
      next = (lista_ptr)->data + (lista_ptr)->len;                \
      ++((lista_ptr)->len);                                       \
    } else {                                                      \
      next = NULL;                                                \
    }                                                             \
    next;                                                         \
  })

// Pushes an item onto the lista, expanding the memory if necessary.
// Returns a pointer to the new item, or NULL if allocation failed.
// Uses default 2x geometric growth.
#define ac_lista_next_ex(lista_ptr) ac_lista_next_exg(lista_ptr, 2, 0, 1)

// All basic data types.
ac_lista_define_type(float);
ac_lista_define_type(double);
ac_lista_define_type(char);
ac_lista_define_type(int8_t);
ac_lista_define_type(int16_t);
ac_lista_define_type(int32_t);
ac_lista_define_type(int64_t);
ac_lista_define_type(uint8_t);
ac_lista_define_type(uint16_t);
ac_lista_define_type(uint32_t);
ac_lista_define_type(uint64_t);
ac_lista_define_type(size_t);

#endif  // AC_ALLOC_H_
