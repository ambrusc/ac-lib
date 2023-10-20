// GZip header parsing, inflation using miniz.

#ifndef AC_GZIP_H_
#define AC_GZIP_H_

#include "ac_mem.h"

// Miniz is optional, but including it provides the 'inflate' function.
#ifndef AC_GZIP_NO_MINIZ
#include "miniz/miniz.h"
#endif  // AC_GZIP_NO_MINIZ

enum : uint16_t {
  AC_GZIP_MAGIC = 0x8b1f,  // Little-endian.
};

enum : uint8_t { AC_GZIP_COMPRESSION_DEFLATE = 0x8 };

// True if the file begins with the GZip magic DEFLATE compression.
static inline bool ac_gzip_magic_match(ac_buf file);

enum : uint8_t {
  // If set, hints that the output file is text.
  AC_GZIP_FLAG_TEXT = 0x01,
  // 16-bit header CRC is present immediately before the compressed data.
  AC_GZIP_FLAG_HCRC = 0x02,
  // "Extra" data field present after header.
  AC_GZIP_FLAG_EXTRA = 0x04,
  // ISO 8859-1 Latin-1 zero-terminated name after any "extra" fields.
  AC_GZIP_FLAG_NAME = 0x08,
  // ISO 8859-1 Latin-1 zero-terminated comment after the "name" field.
  AC_GZIP_FLAG_COMMENT = 0x10,
  // Remaining 3 bits reserved.
};

// OS ID values.
enum : uint8_t {
  AC_GZIP_OS_FAT = 0,
  AC_GZIP_OS_AMIGA = 1,
  AC_GZIP_OS_VMS = 2,
  AC_GZIP_OS_UNIX = 3,
  AC_GZIP_OS_VM = 4,
  AC_GZIP_OS_ATARI = 5,
  AC_GZIP_OS_HPFS = 6,
  AC_GZIP_OS_MACINTOSH = 7,
  AC_GZIP_OS_ZSYSTEM = 8,
  AC_GZIP_OS_CPM = 9,
  AC_GZIP_OS_TOPS20 = 10,
  AC_GZIP_OS_NTFS = 11,
  AC_GZIP_OS_QDOS = 12,
  AC_GZIP_OS_ACORN_RISCOS = 13,
  AC_GZIP_OS_UNKNOWN = 255,
};

// Standard GZip header.
typedef struct __attribute__((__packed__)) ac_gzip_header {
  uint16_t magic;       // AC_GZIP_MAGIC 1f 8b
  uint8_t compression;  // AC_GZIP_COMPRESSION_... 08 -> deflate.
  uint8_t flags;        // AC_GZIP_FLAGS_...
  uint32_t modified_time;
  uint8_t compression_flags;  // deflate: 0x02 best, 0x04 fastest.
  uint8_t os_id;              // AC_GZIP_OS_ID
} ac_gzip_header;
_Static_assert(sizeof(ac_gzip_header) == 10, "");

// Standard GZip footer.
typedef struct ac_gzip_footer {
  uint32_t crc;
  uint32_t decompressed_size;
} ac_gzip_footer;
_Static_assert(sizeof(ac_gzip_footer) == 8, "");

// Index for a GZip file.
typedef struct ac_gzip {
  // Original file buffer.
  ac_buf buffer;

  // Header and associated optional/variable-length data.
  ac_gzip_header header;
  ac_buf extra;         // May be empty.
  const char* name;     // May be null.
  const char* comment;  // May be null.
  uint16_t header_crc;  // Zero if not present in file.

  // Compressed data and footer (points into 'buffer').
  ac_buf rest;

  // If the file has been inflated, footer data is filled out, otherwise zero.
  ac_gzip_footer footer;
} ac_gzip;

// Inits a gzip structure for this file.
//  - Returns 'true' if the index is consistent, 'false' otherwise.
//  - 'file' memory must outlive 'gzip'.
static inline bool ac_gzip_init(ac_gzip* gzip, ac_buf file);

#ifndef AC_GZIP_NO_MINIZ
// Inflates the zipped archive contents into an expanding buffer.
static inline void ac_gzip_inflate(ac_gzip* gzip, ac_list(uint8_t) * out,
                                   ac_allocator alloc);
#endif  // AC_GZIP_NO_MINIZ

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

static inline bool ac_gzip_magic_match(ac_buf file) {
  if (file.size < 3) return false;
  return *(const uint16_t*)file.data == AC_GZIP_MAGIC;
  return *(const uint8_t*)(file.data + 2) == AC_GZIP_COMPRESSION_DEFLATE;
}

static inline bool ac_gzip_init(ac_gzip* gzip, ac_buf file) {
  // Clear the output.
  *gzip = (ac_gzip){};

  // Check for magic match and sufficiently large file.
  if (file.size < sizeof(ac_gzip_header)) return false;
  if (!ac_gzip_magic_match(file)) return false;
  size_t offset = sizeof(ac_gzip_header);

  // Header.
  const ac_gzip_header* header = (const ac_gzip_header*)file.data;

  // Extras.
  ac_buf extra = {};
  if (header->flags & AC_GZIP_FLAG_EXTRA) {
    if (offset + 2 >= file.size) return false;
    const uint16_t extra_size = *(const uint16_t*)file.data + offset;
    offset += 2;

    extra = (ac_buf){file.data + offset, extra_size};
    offset += extra_size;
    if (offset >= file.size) return false;
  }

  // Name.
  const char* name = NULL;
  if (header->flags & AC_GZIP_FLAG_NAME) {
    name = (const char*)(file.data + offset);
    for (; offset < file.size && file.data[offset]; ++offset) {
    }
    ++offset;  // Null terminator.
    if (offset >= file.size) return false;
  }

  // Comment.
  const char* comment = NULL;
  if (header->flags & AC_GZIP_FLAG_COMMENT) {
    comment = (const char*)(file.data + offset);
    for (; offset < file.size && file.data[offset]; ++offset) {
    }
    ++offset;  // Null terminator.
    if (offset >= file.size) return false;
  }

  // Header CRC.
  uint16_t hcrc = 0;
  if (header->flags & AC_GZIP_FLAG_HCRC) {
    if (offset + 2 >= file.size) return false;
    hcrc = *(const uint16_t*)file.data + offset;
    offset += 2;
  }

  // Rest of the file is compressed + footer.
  if (offset >= file.size) return false;
  ac_buf rest = (ac_buf){file.data + offset, file.size - offset};

  // Set the output.
  *gzip = (ac_gzip){
      .buffer = file,
      .header = *header,
      .extra = extra,
      .name = name,
      .comment = comment,
      .header_crc = hcrc,
      .rest = rest,
  };
  return true;
}

#ifndef AC_GZIP_NO_MINIZ
static inline void ac_gzip_inflate(ac_gzip* gzip, ac_list(uint8_t) * out,
                                   ac_allocator alloc) {
  if (gzip->rest.size < sizeof(ac_gzip_footer)) return;

  // Clear/allocate the output, guess a size.
  out->len = 0;
  if (out->cap < 2 * gzip->rest.size) {
    ac_list_realloc(out, &alloc, 2 * gzip->rest.size);
  }

  // Init the inflation stream.
  mz_stream stream = {};
  stream.next_in = gzip->rest.data;
  stream.avail_in = gzip->rest.size;
  stream.next_out = out->data;
  stream.avail_out = out->cap;

  // Negative window bits indicates NO zlib headers (raw deflate stream).
  // GZip files have GZip headers, which we've already read.
  if (mz_inflateInit2(&stream, -MZ_DEFAULT_WINDOW_BITS) != MZ_OK) {
    fprintf(stderr, "miniz init failed\n");
    return;
  }

  // No zlib headers (we read them already).
  while (true) {
    // Inflate the next chunk.
    const int status = mz_inflate(&stream, MZ_SYNC_FLUSH);

    // Done.
    if (status == MZ_STREAM_END) break;

    // Something went wrong.
    if (status != MZ_OK) {
      fprintf(stderr, "miniz error:%d in:%zu out:%zu\n", status,
              (size_t)stream.total_in, (size_t)stream.total_out);
      mz_inflateEnd(&stream);
      return;
    }

    // Input is exhausted (file is trucated).
    if (!stream.avail_in) {
      fprintf(stderr, "miniz input exhausted\n");
      mz_inflateEnd(&stream);
      return;
    }

    // Output is full. Reallocate.
    if (!stream.avail_out) {
      out->len = out->cap;
      const size_t offset = out->cap;
      ac_list_realloc(out, &alloc, 2 * out->cap);
      stream.next_out = out->data + offset;
      stream.avail_out = out->cap - offset;
    }
  }

  // Finish up.
  if (mz_inflateEnd(&stream) != MZ_OK) {
    fprintf(stderr, "miniz end failed\n");
    return;
  }
  out->len = stream.total_out;

  // Check if there's room for the footer.
  if (stream.total_in + sizeof(ac_gzip_footer) > gzip->rest.size) {
    fprintf(stderr, "miniz no room for footer\n");
    return;
  }
  gzip->footer = *(const ac_gzip_footer*)(gzip->rest.data + stream.total_in);
}
#endif  // AC_GZIP_NO_MINIZ

#endif  // AC_GZIP_H_
