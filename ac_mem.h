#ifndef AC_MEM_H_
#define AC_MEM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ac_math.h"

//------------------------------------------------------------------------------
// Utils.
//------------------------------------------------------------------------------

#define ac_array_len(array) (sizeof(array) / sizeof(__typeof__(*array)))

// Returns 'offset' rounded up to the next multiple of 'align'.
static inline size_t ac_align_up(size_t offset, size_t align) {
  return (offset + (align - 1)) / align * align;
}

//------------------------------------------------------------------------------
// Memory partitioning.
//------------------------------------------------------------------------------

// Helps partition a large slab of memory into blocks.
typedef struct ac_slab {
  size_t offset;  // Current byte into the slab.
} ac_slab;

// One block in a slab.
typedef struct ac_slab_block {
  size_t offset;  // Offset in bytes from the beginning of the slab.
  size_t size;    // Size in bytes of this block.
} ac_slab_block;

// The current size of the slab in bytes.
static inline size_t ac_slab_size(const ac_slab* s) { return s->offset; }

// Add a block to the slab.
static inline ac_slab_block ac_slab_alloc(ac_slab* s, size_t size) {
  const ac_slab_block ret = {.offset = s->offset, .size = size};
  s->offset += size;
  return ret;
}

// Add an aligned block to the slab.
static inline ac_slab_block ac_slab_alloc_aligned(ac_slab* s, size_t align,
                                                  size_t size) {
  s->offset = ac_align_up(s->offset, align);
  return ac_slab_alloc(s, size);
}

// Add an aligned block to the slab, sized to the given type and count.
#define ac_slab_alloc_type(slab, element_type, element_count) \
  ac_slab_alloc_aligned(slab, _Alignof(element_type),         \
                        sizeof(element_type) * element_count)

//------------------------------------------------------------------------------
// Untyped Memory Buffers.
//------------------------------------------------------------------------------

// Arbitrary data buffer.
typedef struct ac_buf {
  unsigned char* data;
  size_t size;
} ac_buf;

// Creates a buffer pointing to a struct.
#define AC_BUF_FROM_STRUCT(ptr) \
  (ac_buf) { .data = (unsigned char*)ptr, .size = sizeof(*ptr) }

// Creates a buffer from a begin/end pointer pair.
//  - If begin > end, size is set to zero.
#define AC_BUF_FROM_RANGE(begin, end)                                       \
  (ac_buf) {                                                                \
    .data = (unsigned char*)begin,                                          \
    .size = (size_t)ac_max(                                                 \
        (int64_t)0, (int64_t)((unsigned char*)end - (unsigned char*)begin)) \
  }

#define ac_buf_get(buf_ptr, val_ptr_ptr)                              \
  ({                                                                  \
    const size_t size = sizeof(**(val_ptr_ptr));                      \
    const bool valid = (buf_ptr)->size >= size;                       \
    if (valid) {                                                      \
      *(val_ptr_ptr) = (__typeof__(*(val_ptr_ptr)))((buf_ptr)->data); \
      (buf_ptr)->data += size;                                        \
      (buf_ptr)->size -= size;                                        \
    }                                                                 \
    valid;                                                            \
  })

#define ac_buf_get_val(buf_ptr, val_ptr)                    \
  ({                                                        \
    const size_t size = sizeof(*val_ptr);                   \
    const bool valid = (buf_ptr)->size >= size;             \
    if (valid) {                                            \
      *(val_ptr) = *(__typeof__(val_ptr))((buf_ptr)->data); \
      (buf_ptr)->data += size;                              \
      (buf_ptr)->size -= size;                              \
    }                                                       \
    valid;                                                  \
  })

#define ac_buf_get_span(buf_ptr, span_ptr, read_len)                      \
  ({                                                                      \
    const size_t size = sizeof(*((span_ptr)->data)) * read_len;           \
    const bool valid = (buf_ptr)->size >= size;                           \
    if (valid) {                                                          \
      (span_ptr)->data = (__typeof__((span_ptr)->data))((buf_ptr)->data); \
      (span_ptr)->len = read_len;                                         \
      (buf_ptr)->data += size;                                            \
      (buf_ptr)->size -= size;                                            \
    }                                                                     \
    valid;                                                                \
  })

//------------------------------------------------------------------------------
// Typed non-resizable buffers.
//------------------------------------------------------------------------------

#define ac_span(element_type) ac_span_##element_type

#define ac_span_define_type(element_type) \
  typedef struct ac_span(element_type) {  \
    element_type* data;                   \
    size_t len;                           \
  }                                       \
  ac_span(element_type)

ac_span_define_type(float);
ac_span_define_type(double);
ac_span_define_type(char);
ac_span_define_type(int8_t);
ac_span_define_type(int16_t);
ac_span_define_type(int32_t);
ac_span_define_type(int64_t);
ac_span_define_type(uint8_t);
ac_span_define_type(uint16_t);
ac_span_define_type(uint32_t);
ac_span_define_type(uint64_t);
ac_span_define_type(size_t);

#define ac_span_from_array(element_type, arr)                 \
  (ac_span(element_type)) {                                   \
    .data = arr, .len = (sizeof(arr) / sizeof(element_type)), \
  }

#define ac_span_from_slab(element_type, mem, block)     \
  (ac_span(element_type)) {                             \
    .data = (element_type*)((char*)mem + block.offset), \
    .len = block.size / sizeof(element_type),           \
  }

#define ac_span_copy(dst, src) \
  memcpy(                      \
      dst.data, src.data,      \
      ac_min((sizeof(*dst.data) * dst.len), (sizeof(*src.data) * src.len)));

//------------------------------------------------------------------------------
// Typed immutable non-resizable buffers.
//------------------------------------------------------------------------------

#define ac_cspan(element_type) ac_cspan_##element_type

#define ac_cspan_define_type(element_type) \
  typedef struct ac_cspan(element_type) {  \
    const element_type* data;              \
    size_t len;                            \
  }                                        \
  ac_cspan(element_type)

ac_cspan_define_type(float);
ac_cspan_define_type(double);
ac_cspan_define_type(char);
ac_cspan_define_type(int8_t);
ac_cspan_define_type(int16_t);
ac_cspan_define_type(int32_t);
ac_cspan_define_type(int64_t);
ac_cspan_define_type(uint8_t);
ac_cspan_define_type(uint16_t);
ac_cspan_define_type(uint32_t);
ac_cspan_define_type(uint64_t);
ac_cspan_define_type(size_t);

#define ac_cspan_from_array(element_type, arr)                \
  (ac_cspan(element_type)) {                                  \
    .data = arr, .len = (sizeof(arr) / sizeof(element_type)), \
  }

#define ac_cspan_from_slab(element_type, mem, block)    \
  (ac_cspan(element_type)) {                            \
    .data = (element_type*)((char*)mem + block.offset), \
    .len = block.size / sizeof(element_type),           \
  }

//------------------------------------------------------------------------------
// Basic Allocation.
//------------------------------------------------------------------------------

typedef void* (*ac_alloc_fn)(void* state, size_t size);
typedef void (*ac_free_fn)(void* state, void* ptr);

typedef struct ac_allocator {
  void* state;
  ac_alloc_fn alloc;
  ac_free_fn free;
} ac_allocator;

static inline void* ac_alloc(ac_allocator* a, size_t size) {
  return a->alloc(a->state, size);
}

static inline void ac_free(ac_allocator* a, void* ptr) {
  if (a->free) a->free(a->state, ptr);
}

//------------------------------------------------------------------------------
// Mallocation Allocation.
//------------------------------------------------------------------------------

static inline void* ac_call_malloc(void* state, size_t size) {
  (void)state;
  return malloc(size);
}

static inline void ac_call_free(void* state, void* ptr) {
  (void)state;
  if (ptr) free(ptr);
}

static inline ac_allocator ac_mallocator() {
  return (ac_allocator){
      .alloc = &ac_call_malloc,
      .free = &ac_call_free,
  };
}

//------------------------------------------------------------------------------
// Arena Allocation.
//------------------------------------------------------------------------------

// An arena allocates in large blocks of 'alloc_size' and divides it into
// smaller allocations requested by the caller. With properly selected
// 'alloc_size', it reduces the number of calls to malloc (performance) and is
// free'd all at once (ease of use).
//
// Allocations are usually made from the current block.
// Entire arena must be destroyed at once (individual allocs cannot be free'd).
// If the current block cannot fit the next 'alloc', a new block is allocated
//   and made current.
// Memory is only ever allocated from the current block, EXCEPT
// Allocations of 'alloc_size' / 2 receive their own block, leaving the current
//   block unchaged.
//   - This reduces possible fragmentation AND
//   - Allows the caller to request allocations larger than 'alloc_size'.

// Alloc/free functions. Same semantics as malloc/free.
typedef void* (*ac_arena_alloc_fn)(size_t);
typedef void (*ac_arena_free_fn)(void*);

// Arena allocation options.
typedef struct ac_arena_opts {
  size_t alloc_size;  // Minimum size of each allocation.
  ac_arena_alloc_fn alloc;
  ac_arena_free_fn free;
} ac_arena_opts;

// One node in the arena linked list.
typedef struct ac_arena_node {
  unsigned char* data;
  size_t size;
  size_t pos;
} ac_arena_node;

// Arena allocation state.
typedef struct ac_arena {
  ac_arena_opts opts;
  ac_arena_node root;
} ac_arena;

// Initialize a memory arena which allocates in blocks of 'alloc_size'.
// Defaults are used for zero-initialized fields of opts.
//   - alloc_size => 1 MiB
//   - alloc => malloc
//   - free => free
ac_arena ac_arena_create(ac_arena_opts);

// Allocate memory from the given arena.
void* ac_arena_alloc(ac_arena*, size_t);
ac_buf ac_arena_alloc_buf(ac_arena*, size_t);

// Free the entire arena.
void ac_arena_destroy(ac_arena*);

//------------------------------------------------------------------------------
// Contiguous buffers (lists) with capacity and size.
//------------------------------------------------------------------------------

#define ac_list(element_type) ac_list_##element_type

#define ac_list_define_type(element_type) \
  typedef struct ac_list(element_type) {  \
    element_type* data;                   \
    size_t len;                           \
    size_t cap;                           \
  }                                       \
  ac_list(element_type)

#define ac_list_from_slab(element_type, mem, block)     \
  (ac_list(element_type)) {                             \
    .data = (element_type*)((char*)mem + block.offset), \
    .cap = block.size / sizeof(element_type),           \
  }

#define ac_list_realloc(list_ptr, alloc_ptr, new_cap)                   \
  do {                                                                  \
    const size_t element_size = sizeof(__typeof__(*(list_ptr)->data));  \
    void* const new_data = ac_alloc(alloc_ptr, element_size * new_cap); \
    const size_t new_len = ac_min((list_ptr)->len, new_cap);            \
    const size_t copy_size = element_size * new_len;                    \
    if (new_data && (list_ptr)->data && copy_size) {                    \
      memcpy(new_data, (list_ptr)->data, copy_size);                    \
    }                                                                   \
    ac_free(alloc_ptr, (list_ptr)->data);                               \
    (list_ptr)->data = new_data;                                        \
    (list_ptr)->len = new_len;                                          \
    (list_ptr)->cap = new_cap;                                          \
  } while (false)

// Pushes onto a list without bounds checking.
#define ac_list_push(list_ptr, value) \
  (list_ptr)->data[(list_ptr)->len++] = value

// Deletes the i-th value from a list without bounds checking.
//  - Swaps the last element into the deleted element's spot.
#define ac_list_del_nth(list_ptr, i)                               \
  do {                                                             \
    if (i < (list_ptr)->len - 1) {                                 \
      (list_ptr)->data[i] = (list_ptr)->data[(list_ptr)->len - 1]; \
    }                                                              \
    --(list_ptr)->len;                                             \
  } while (false)

// Deletes the first instance of a value from the list.
// Returns 'true' if the value was found, false otherwise.
#define ac_list_del_value(list_ptr, value)         \
  ({                                               \
    bool _found_value = false;                     \
    for (size_t i = 0; i < (list_ptr)->len; ++i) { \
      if ((list_ptr)->data[i] == (value)) {        \
        _found_value = true;                       \
        ac_list_del_nth((list_ptr), (i));          \
        break;                                     \
      }                                            \
    }                                              \
    _found_value;                                  \
  })

ac_list_define_type(float);
ac_list_define_type(double);
ac_list_define_type(char);
ac_list_define_type(int8_t);
ac_list_define_type(int16_t);
ac_list_define_type(int32_t);
ac_list_define_type(int64_t);
ac_list_define_type(uint8_t);
ac_list_define_type(uint16_t);
ac_list_define_type(uint32_t);
ac_list_define_type(uint64_t);
ac_list_define_type(size_t);

//------------------------------------------------------------------------------
// File mapping for read.
//------------------------------------------------------------------------------

// Returns a mapped file or NULL/0-size if the mapping failed for any reason.
ac_buf ac_file_map_read(const char* path);

// Unmaps the buffer if it's non-NULL and has nonzero size.
void ac_file_unmap(ac_buf buf);

//------------------------------------------------------------------------------
// Implementation.
//------------------------------------------------------------------------------
#endif  // AC_MEM_H_

#if defined(AC_MEM_IMPL)
#ifndef AC_MEM_H_IMPL_
#define AC_MEM_H_IMPL_

#if defined(_WIN32)

// TODO(ambrus) at least compile with MinGW.

#else  // NOT WINDOWS

ac_arena ac_arena_create(ac_arena_opts x) {
  if (!x.alloc_size) x.alloc_size = 1024 * 1024;
  if (!x.alloc) x.alloc = &malloc;
  if (!x.free) x.free = &free;

  const size_t size = sizeof(ac_arena_node) + x.alloc_size;
  ac_arena_node* next = x.alloc(size);
  *next = (ac_arena_node){};

  return (ac_arena){
      .opts = x,
      .root = {.data = (unsigned char*)next,
               .size = size,
               .pos = sizeof(ac_arena_node)},
  };
}

void* ac_arena_alloc(ac_arena* x, size_t s) {
  // Allocate from the root buffer.
  if (x->root.pos + s < x->root.size) {
    void* ret = x->root.data + x->root.pos;
    x->root.pos += s;
    return ret;
  }

  // Large allocations directly become the second node.
  if (s > x->opts.alloc_size / 2) {
    const size_t size = sizeof(ac_arena_node) + s;

    // Point the new block at the third block.
    ac_arena_node* node = x->opts.alloc(size);
    *node = *(ac_arena_node*)x->root.data;

    // Point the second block at the new block.
    // (Root still points to the second block, which may still have plenty of
    // space, just not enough for such a large allocation).
    *(ac_arena_node*)x->root.data = (ac_arena_node){
        .data = (unsigned char*)node,
        .size = size,
        .pos = sizeof(ac_arena_node) + s,
    };

    // Return the new allocation.
    return (unsigned char*)node + sizeof(ac_arena_node);
  }

  // If there's no space in the buffer, allocate another block.
  const size_t size = sizeof(ac_arena_node) + x->opts.alloc_size;
  ac_arena_node* node = x->opts.alloc(size);

  // Point the new block at the first block.
  *node = x->root;

  // Point root at the new block.
  x->root = (ac_arena_node){
      .data = (unsigned char*)node,
      .size = size,
      .pos = sizeof(ac_arena_node) + s,
  };

  // Return the new allocation.
  return (unsigned char*)node + sizeof(ac_arena_node);
}

ac_buf ac_arena_alloc_buf(ac_arena* x, size_t s) {
  return (ac_buf){(unsigned char*)ac_arena_alloc(x, s), s};
}

void ac_arena_destroy(ac_arena* x) {
  void* data = x->root.data;
  do {
    void* next = ((ac_arena_node*)data)->data;
    free(data);
    data = next;
  } while (data);
  // Don't write to 'x' again, as it might have just been free'd.
}

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

ac_buf ac_file_map_read(const char* path) {
  const int fd = open(path, O_RDONLY | O_NONBLOCK);
  if (fd < 0) return (ac_buf){};

  const size_t size = lseek(fd, 0, SEEK_END);

  void* data = mmap(/*addr=*/0, size, PROT_READ, MAP_PRIVATE, fd, /*offset=*/0);
  close(fd);

  if (data < 0) return (ac_buf){};
  return (ac_buf){.data = data, .size = size};
}

void ac_file_unmap(ac_buf buf) {
  if (buf.data && buf.size) munmap(buf.data, buf.size);
}

#endif  // NOT WINDOWS

#endif  // A_MEM_H_IMPL_
#endif  // AC_MEM_IMPL
