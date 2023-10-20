#ifndef AC_VEC_H_
#define AC_VEC_H_

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct ac_v2f {
  float x, y;
} ac_v2f;

typedef struct ac_v3f {
  float x, y, z;
} ac_v3f;

typedef struct ac_v4f {
  float x, y, z, w;
} ac_v4f;

typedef struct ac_mat4f {
  ac_v4f col[4];  // Column-major memory order.
} ac_mat4f;

// Prints in memory order (transposed).
static inline void ac_v2f_print(FILE* f, ac_v2f a);
static inline void ac_v3f_print(FILE* f, ac_v3f a);
static inline void ac_v4f_print(FILE* f, ac_v4f a);
static inline void ac_mat4f_print(FILE* f, ac_mat4f a);

// Prints as column vectors (logical shape/order).
static inline void ac_v2f_vprint(FILE* f, ac_v2f a);
static inline void ac_v3f_vprint(FILE* f, ac_v3f a);
static inline void ac_v4f_vprint(FILE* f, ac_v4f a);
static inline void ac_mat4f_vprint(FILE* f, ac_mat4f a);

// 2-element vector.
static inline ac_v2f ac_v2f_x();
static inline ac_v2f ac_v2f_y();
static inline bool ac_v2f_eq(ac_v2f a, ac_v2f b);
static inline float ac_v2f_dot(ac_v2f a, ac_v2f b);
static inline float ac_v2f_cross(ac_v2f a, ac_v2f b);
static inline float ac_v2f_len2(ac_v2f a);
static inline ac_v2f ac_v2f_normed(ac_v2f a);
static inline ac_v2f ac_v2f_neg(ac_v2f a);
static inline ac_v2f ac_v2f_add(ac_v2f a, ac_v2f b);
static inline ac_v2f ac_v2f_sub(ac_v2f a, ac_v2f b);
static inline ac_v2f ac_v2f_scale(ac_v2f a, float s);
static inline ac_v2f ac_v2f_min(ac_v2f a, ac_v2f b);
static inline ac_v2f ac_v2f_max(ac_v2f a, ac_v2f b);

// 3-element vector.
static inline ac_v3f ac_v3f_x();
static inline ac_v3f ac_v3f_y();
static inline ac_v3f ac_v3f_z();
static inline bool ac_v3f_eq(ac_v3f a, ac_v3f b);
static inline float ac_v3f_dot(ac_v3f a, ac_v3f b);
static inline float ac_v3f_len2(ac_v3f a);
static inline ac_v3f ac_v3f_normed(ac_v3f a);
static inline ac_v3f ac_v3f_cross(ac_v3f a, ac_v3f b);
static inline ac_v3f ac_v3f_neg(ac_v3f a);
static inline ac_v3f ac_v3f_add(ac_v3f a, ac_v3f b);
static inline ac_v3f ac_v3f_sub(ac_v3f a, ac_v3f b);
static inline ac_v3f ac_v3f_scale(ac_v3f a, float s);
static inline ac_v3f ac_v3f_min(ac_v3f a, ac_v3f b);
static inline ac_v3f ac_v3f_max(ac_v3f a, ac_v3f b);

// 4-element vector.
static inline ac_v4f ac_v4f_x();
static inline ac_v4f ac_v4f_y();
static inline ac_v4f ac_v4f_z();
static inline ac_v4f ac_v4f_w();
static inline bool ac_v4f_eq(ac_v4f a, ac_v4f b);
static inline float ac_v4f_dot(ac_v4f a, ac_v4f b);
static inline float ac_v4f_len2(ac_v4f a);
static inline ac_v4f ac_v4f_normed(ac_v4f a);
static inline ac_v4f ac_v4f_neg(ac_v4f a);
static inline ac_v4f ac_v4f_add(ac_v4f a, ac_v4f b);
static inline ac_v4f ac_v4f_sub(ac_v4f a, ac_v4f b);
static inline ac_v4f ac_v4f_scale(ac_v4f a, float s);
static inline ac_v4f ac_v4f_min(ac_v4f a, ac_v4f b);
static inline ac_v4f ac_v4f_max(ac_v4f a, ac_v4f b);

// 4x4 matrix.
static inline ac_mat4f ac_mat4f_I();
static inline ac_v4f ac_mat4f_row(ac_mat4f a, size_t i);
static inline ac_mat4f ac_mat4f_ortho(float left, float right, float bot,
                                      float top, float near, float far);
static inline ac_mat4f ac_mat4f_trn(ac_v3f a);
static inline ac_mat4f ac_mat4f_rot_cs(ac_v3f a, float ct, float st);
static inline ac_mat4f ac_mat4f_rot(ac_v3f a, float th);
static inline ac_mat4f ac_mat4f_rotx_cs(float ct, float st);
static inline ac_mat4f ac_mat4f_roty_cs(float ct, float st);
static inline ac_mat4f ac_mat4f_rotz_cs(float ct, float st);
static inline ac_mat4f ac_mat4f_rotx(float th);
static inline ac_mat4f ac_mat4f_roty(float th);
static inline ac_mat4f ac_mat4f_rotz(float th);
static inline ac_mat4f ac_mat4f_lookat(ac_v3f eye, ac_v3f center, ac_v3f up);
static inline ac_mat4f ac_mat4f_mul(ac_mat4f a, ac_mat4f b);
static inline ac_v4f ac_mat4f_mulv(ac_mat4f a, ac_v4f b);

// Octahedral mapping.
//  - 3-vector on unit sphere <=> 2-vector on the range [-1, 1].
//  - No precondition checking: garbage-in garbage-out.
static inline ac_v2f ac_v3f_to_oct(ac_v3f a);
static inline ac_v3f ac_oct_to_v3f(ac_v2f a);

// Line-line intersection (nearest point) in 3-space.
//   p: Starting point of each segment.
//   v: Direction and length of each segment (p + v is the segment endpoint).
// Solves the system of equations:
//   s0 * v0 + p0 = s1 * v1 + p1
// representing the intersection of the two lines, in the least-squares sense.
// Returns the vector [s0, s1].
//   Because 's' scales 'v': 's' on the range [0, 1] indicates that the nearest
//   point is within the segment boundaries.
//   If 'v' is unit, then 's' is a distance along the segment from 'p'.
static inline ac_v2f ac_llint_3f(ac_v3f p0, ac_v3f v0, ac_v3f p1, ac_v3f v1);

// Line-line distance (nearest point) in 3-space.
//   p: Starting point of each segment.
//   v: Direction and length of each segment (p + v is the segment endpoint).
// Solves the system of equations:
//   s0 * v0 + p0 = s1 * v1 + p1
// representing the intersection of the two lines, in the least-squares sense,
// and then the equation:
//   d = |(s0 * v0 + p0) - (s1 * v1 + p1)|^2
// Returns the vector [s0, s1, d]
static inline ac_v3f ac_lldist2_3f(ac_v3f p0, ac_v3f v0, ac_v3f p1, ac_v3f v1);

//------------------------------------------------------------------------------
// Implementation.
//------------------------------------------------------------------------------

static inline void ac_v2f_print(FILE* f, ac_v2f a) {
  fprintf(f, "(ac_v2f){%f, %f}", a.x, a.y);
}

static inline void ac_v3f_print(FILE* f, ac_v3f a) {
  fprintf(f, "(ac_v3f){%f, %f, %f}", a.x, a.y, a.z);
}

static inline void ac_v4f_print(FILE* f, ac_v4f a) {
  fprintf(f, "(ac_v4f){%f, %f, %f, %f}", a.x, a.y, a.z, a.w);
}

static inline void ac_mat4f_print(FILE* f, ac_mat4f a) {
  fprintf(f, "(ac_mat4f){\n  ");
  ac_v4f_print(f, a.col[0]);
  fprintf(f, ",\n  ");
  ac_v4f_print(f, a.col[1]);
  fprintf(f, ",\n  ");
  ac_v4f_print(f, a.col[2]);
  fprintf(f, ",\n  ");
  ac_v4f_print(f, a.col[3]);
  fprintf(f, "}");
}

static inline void ac_v2f_vprint(FILE* f, ac_v2f a) {
  fprintf(f, "[%f]\n[%f]", a.x, a.y);
}

static inline void ac_v3f_vprint(FILE* f, ac_v3f a) {
  fprintf(f, "[%f]\n[%f]\n[%f]\n", a.x, a.y, a.z);
}

static inline void ac_v4f_vprint(FILE* f, ac_v4f a) {
  fprintf(f, "[%f]\n[%f]\n[%f]\n[%f]\n", a.x, a.y, a.z, a.w);
}

static inline void ac_mat4f_vprint(FILE* f, ac_mat4f a) {
  fprintf(f, "[%f %f %f %f]\n", a.col[0].x, a.col[1].x, a.col[2].x, a.col[3].x);
  fprintf(f, "[%f %f %f %f]\n", a.col[0].y, a.col[1].y, a.col[2].y, a.col[3].y);
  fprintf(f, "[%f %f %f %f]\n", a.col[0].z, a.col[1].z, a.col[2].z, a.col[3].z);
  fprintf(f, "[%f %f %f %f]\n", a.col[0].w, a.col[1].w, a.col[2].w, a.col[3].w);
}

//------------------------------------------------------------------------------
// Unit direction vectors.
//------------------------------------------------------------------------------

static inline ac_v2f ac_v2f_x() { return (ac_v2f){1, 0}; }
static inline ac_v2f ac_v2f_y() { return (ac_v2f){0, 1}; }

static inline ac_v3f ac_v3f_x() { return (ac_v3f){1, 0, 0}; }
static inline ac_v3f ac_v3f_y() { return (ac_v3f){0, 1, 0}; }
static inline ac_v3f ac_v3f_z() { return (ac_v3f){0, 0, 1}; }

static inline ac_v4f ac_v4f_x() { return (ac_v4f){1, 0, 0, 0}; }
static inline ac_v4f ac_v4f_y() { return (ac_v4f){0, 1, 0, 0}; }
static inline ac_v4f ac_v4f_z() { return (ac_v4f){0, 0, 1, 0}; }
static inline ac_v4f ac_v4f_w() { return (ac_v4f){0, 0, 0, 1}; }

//------------------------------------------------------------------------------
// Vector math.
//------------------------------------------------------------------------------

static inline bool ac_v2f_eq(ac_v2f a, ac_v2f b) {
  return (a.x == b.x) & (a.y == b.y);
}
static inline float ac_v2f_dot(ac_v2f a, ac_v2f b) {
  return a.x * b.x + a.y * b.y;
}
static inline float ac_v2f_cross(ac_v2f a, ac_v2f b) {
  return a.x * b.y - a.y * b.x;
}
static inline float ac_v2f_len2(ac_v2f a) { return ac_v2f_dot(a, a); }
static inline ac_v2f ac_v2f_normed(ac_v2f a) {
  return ac_v2f_scale(a, 1.f / sqrtf(ac_v2f_len2(a)));
}
static inline ac_v2f ac_v2f_neg(ac_v2f a) { return (ac_v2f){-a.x, -a.y}; }
static inline ac_v2f ac_v2f_add(ac_v2f a, ac_v2f b) {
  return (ac_v2f){a.x + b.x, a.y + b.y};
}
static inline ac_v2f ac_v2f_sub(ac_v2f a, ac_v2f b) {
  return (ac_v2f){a.x - b.x, a.y - b.y};
}
static inline ac_v2f ac_v2f_scale(ac_v2f a, float s) {
  return (ac_v2f){a.x * s, a.y * s};
}
static inline ac_v2f ac_v2f_min(ac_v2f a, ac_v2f b) {
  return (ac_v2f){ac_min(a.x, b.x), ac_min(a.y, b.y)};
}
static inline ac_v2f ac_v2f_max(ac_v2f a, ac_v2f b) {
  return (ac_v2f){ac_max(a.x, b.x), ac_max(a.y, b.y)};
}

static inline bool ac_v3f_eq(ac_v3f a, ac_v3f b) {
  return (a.x == b.x) & (a.y == b.y) & (a.z == b.z);
}
static inline float ac_v3f_dot(ac_v3f a, ac_v3f b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
static inline float ac_v3f_len2(ac_v3f a) { return ac_v3f_dot(a, a); }
static inline ac_v3f ac_v3f_normed(ac_v3f a) {
  return ac_v3f_scale(a, 1.f / sqrtf(ac_v3f_len2(a)));
}
static inline ac_v3f ac_v3f_cross(ac_v3f a, ac_v3f b) {
  return (ac_v3f){a.y * b.z - a.z * b.y,  //
                  a.z * b.x - a.x * b.z,  //
                  a.x * b.y - a.y * b.x};
}
static inline ac_v3f ac_v3f_neg(ac_v3f a) { return (ac_v3f){-a.x, -a.y, -a.z}; }
static inline ac_v3f ac_v3f_add(ac_v3f a, ac_v3f b) {
  return (ac_v3f){a.x + b.x, a.y + b.y, a.z + b.z};
}
static inline ac_v3f ac_v3f_sub(ac_v3f a, ac_v3f b) {
  return (ac_v3f){a.x - b.x, a.y - b.y, a.z - b.z};
}
static inline ac_v3f ac_v3f_scale(ac_v3f a, float s) {
  return (ac_v3f){a.x * s, a.y * s, a.z * s};
}
static inline ac_v3f ac_v3f_min(ac_v3f a, ac_v3f b) {
  return (ac_v3f){ac_min(a.x, b.x), ac_min(a.y, b.y), ac_min(a.z, b.z)};
}
static inline ac_v3f ac_v3f_max(ac_v3f a, ac_v3f b) {
  return (ac_v3f){ac_max(a.x, b.x), ac_max(a.y, b.y), ac_max(a.z, b.z)};
}

static inline bool ac_v4f_eq(ac_v4f a, ac_v4f b) {
  return ((a.x == b.x) & (a.y == b.y)) & ((a.z == b.z) & (a.w == b.w));
}
static inline float ac_v4f_dot(ac_v4f a, ac_v4f b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
static inline float ac_v4f_len2(ac_v4f a) { return ac_v4f_dot(a, a); }
static inline ac_v4f ac_v4f_normed(ac_v4f a) {
  return ac_v4f_scale(a, 1.f / sqrtf(ac_v4f_len2(a)));
}
static inline ac_v4f ac_v4f_neg(ac_v4f a) {
  return (ac_v4f){-a.x, -a.y, -a.z, -a.w};
}
static inline ac_v4f ac_v4f_add(ac_v4f a, ac_v4f b) {
  return (ac_v4f){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
static inline ac_v4f ac_v4f_sub(ac_v4f a, ac_v4f b) {
  return (ac_v4f){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
static inline ac_v4f ac_v4f_scale(ac_v4f a, float s) {
  return (ac_v4f){a.x * s, a.y * s, a.z * s, a.w * s};
}
static inline ac_v4f ac_v4f_min(ac_v4f a, ac_v4f b) {
  return (ac_v4f){ac_min(a.x, b.x), ac_min(a.y, b.y), ac_min(a.z, b.z),
                  ac_min(a.w, b.w)};
}
static inline ac_v4f ac_v4f_max(ac_v4f a, ac_v4f b) {
  return (ac_v4f){ac_max(a.x, b.x), ac_max(a.y, b.y), ac_max(a.z, b.z),
                  ac_max(a.w, b.w)};
}

//------------------------------------------------------------------------------
// Matrix math.
//------------------------------------------------------------------------------

static inline ac_mat4f ac_mat4f_I() {
  return (ac_mat4f){{ac_v4f_x(), ac_v4f_y(), ac_v4f_z(), ac_v4f_w()}};
}

static inline ac_v4f ac_mat4f_row(ac_mat4f a, size_t i) {
  switch (i) {
    case 0:
      return (ac_v4f){a.col[0].x, a.col[1].x, a.col[2].x, a.col[3].x};
    case 1:
      return (ac_v4f){a.col[0].y, a.col[1].y, a.col[2].y, a.col[3].y};
    case 2:
      return (ac_v4f){a.col[0].z, a.col[1].z, a.col[2].z, a.col[3].z};
    case 3:
      return (ac_v4f){a.col[0].w, a.col[1].w, a.col[2].w, a.col[3].w};
  }
  return (ac_v4f){};
}

static inline ac_mat4f ac_mat4f_ortho(float left, float right, float bot,
                                      float top, float near, float far) {
  const float xx = 2.0f / (right - left);
  const float yy = 2.0f / (top - bot);
  const float zz = 2.0f / (near - far);

  const float tx = -(right + left) / (right - left);
  const float ty = -(top + bot) / (top - bot);
  const float tz = (near + far) / (near - far);

  return (ac_mat4f){
      (ac_v4f){xx, 0, 0, 0},    //
      (ac_v4f){0, yy, 0, 0},    //
      (ac_v4f){0, 0, zz, 0},    //
      (ac_v4f){tx, ty, tz, 1},  //
  };
}

static inline ac_mat4f ac_mat4f_persp(float fovy, float aspect, float near,
                                      float far) {
  // https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
  const float cotan = 1.0f / tanf(fovy / 2.0f);
  return (ac_mat4f){
      (ac_v4f){cotan / aspect, 0, 0, 0},                     //
      (ac_v4f){0, cotan, 0, 0},                              //
      (ac_v4f){0, 0, (near + far) / (near - far), -1},       //
      (ac_v4f){0, 0, (2.f * near * far) / (near - far), 0},  //
  };
}

static inline ac_mat4f ac_mat4f_trn(ac_v3f a) {
  return (ac_mat4f){
      ac_v4f_x(),                  //
      ac_v4f_y(),                  //
      ac_v4f_z(),                  //
      (ac_v4f){a.x, a.y, a.z, 1},  //
  };
}

static inline ac_mat4f ac_mat4f_rot_cs(ac_v3f a, float ct, float st) {
  const float cc = 1.f - ct;
  const float xs = a.x * st;
  const float ys = a.y * st;
  const float zs = a.z * st;
  const float xyc = a.x * a.y * cc;
  const float xzc = a.x * a.z * cc;
  const float yzc = a.y * a.z * cc;
  const float x2 = ct + cc + a.x * a.x;
  const float y2 = ct + cc * a.y * a.y;
  const float z2 = ct + cc * a.z * a.z;
  return (ac_mat4f){
      // clang-format off
    (ac_v4f){       x2,  zs + xyc, -ys + xzc, 0},
    (ac_v4f){-zs + xyc,        y2,  xs + yzc, 0},
    (ac_v4f){ ys + xzc, -xs + yzc,        z2, 0},
    ac_v4f_w(),
      // clang-format on
  };
}

static inline ac_mat4f ac_mat4f_rot(ac_v3f a, float th) {
  return ac_mat4f_rot_cs(a, cosf(th), sinf(th));
}

static inline ac_mat4f ac_mat4f_rotx_cs(float ct, float st) {
  return (ac_mat4f){
      ac_v4f_x(),               //
      (ac_v4f){0, ct, st, 0},   //
      (ac_v4f){0, -st, ct, 0},  //
      ac_v4f_w(),               //
  };
}

static inline ac_mat4f ac_mat4f_roty_cs(float ct, float st) {
  return (ac_mat4f){
      (ac_v4f){ct, 0, st, 0},   //
      ac_v4f_y(),               //
      (ac_v4f){-st, 0, ct, 0},  //
      ac_v4f_w(),               //
  };
}

static inline ac_mat4f ac_mat4f_rotz_cs(float ct, float st) {
  return (ac_mat4f){
      (ac_v4f){ct, st, 0, 0},   //
      (ac_v4f){-st, ct, 0, 0},  //
      ac_v4f_z(),               //
      ac_v4f_w(),               //
  };
}

static inline ac_mat4f ac_mat4f_rotx(float th) {
  return ac_mat4f_rotx_cs(cosf(th), sinf(th));
}

static inline ac_mat4f ac_mat4f_roty(float th) {
  return ac_mat4f_roty_cs(cosf(th), sinf(th));
}

static inline ac_mat4f ac_mat4f_rotz(float th) {
  return ac_mat4f_rotz_cs(cosf(th), sinf(th));
}

static inline ac_mat4f ac_mat4f_lookat(ac_v3f eye, ac_v3f center, ac_v3f up) {
  const ac_v3f f = ac_v3f_normed(ac_v3f_sub(center, eye));  // Forward.
  const ac_v3f s = ac_v3f_normed(ac_v3f_cross(f, up));      // Right.
  const ac_v3f u = ac_v3f_cross(s, f);  // Up (perp to forward, right).

  return (ac_mat4f){{
      (ac_v4f){s.x, u.x, -f.x, 0},
      (ac_v4f){s.y, u.y, -f.y, 0},
      (ac_v4f){s.z, u.z, -f.z, 0},
      (ac_v4f){-ac_v3f_dot(s, eye), -ac_v3f_dot(u, eye), ac_v3f_dot(f, eye), 1},
  }};
}

static inline ac_mat4f ac_mat4f_mul(ac_mat4f a, ac_mat4f b) {
  const ac_v4f r0 = ac_mat4f_row(a, 0);
  const ac_v4f r1 = ac_mat4f_row(a, 1);
  const ac_v4f r2 = ac_mat4f_row(a, 2);
  const ac_v4f r3 = ac_mat4f_row(a, 3);
  return (ac_mat4f){{
      (ac_v4f){ac_v4f_dot(r0, b.col[0]), ac_v4f_dot(r1, b.col[0]),
               ac_v4f_dot(r2, b.col[0]), ac_v4f_dot(r3, b.col[0])},
      (ac_v4f){ac_v4f_dot(r0, b.col[1]), ac_v4f_dot(r1, b.col[1]),
               ac_v4f_dot(r2, b.col[1]), ac_v4f_dot(r3, b.col[1])},
      (ac_v4f){ac_v4f_dot(r0, b.col[2]), ac_v4f_dot(r1, b.col[2]),
               ac_v4f_dot(r2, b.col[2]), ac_v4f_dot(r3, b.col[2])},
      (ac_v4f){ac_v4f_dot(r0, b.col[3]), ac_v4f_dot(r1, b.col[3]),
               ac_v4f_dot(r2, b.col[3]), ac_v4f_dot(r3, b.col[3])},
  }};
}

static inline ac_v4f ac_mat4f_mulv(ac_mat4f a, ac_v4f b) {
  const ac_v4f r0 = ac_mat4f_row(a, 0);
  const ac_v4f r1 = ac_mat4f_row(a, 1);
  const ac_v4f r2 = ac_mat4f_row(a, 2);
  const ac_v4f r3 = ac_mat4f_row(a, 3);
  return (ac_v4f){ac_v4f_dot(r0, b), ac_v4f_dot(r1, b), ac_v4f_dot(r2, b),
                  ac_v4f_dot(r3, b)};
}

static inline ac_v2f ac_v3f_to_oct(ac_v3f a) {
  // https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
  const float k = 1.f / (fabs(a.x) + fabs(a.y) + fabs(a.z));
  const float kx = k * a.x;
  const float ky = k * a.y;  // Note the x/y flip below ----v
  const float x = (a.z >= 0.f) ? kx : copysignf(1.f - fabs(ky), kx);
  const float y = (a.z >= 0.f) ? ky : copysignf(1.f - fabs(kx), ky);
  return (ac_v2f){x, y};
}

static inline ac_v3f ac_oct_to_v3f(ac_v2f a) {
  // https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
  const float z = 1.f - fabs(a.x) - fabs(a.y);
  const float t = ac_max(-z, 0.f);
  const float x = a.x - copysignf(t, a.x);
  const float y = a.y - copysignf(t, a.y);
  return ac_v3f_normed((ac_v3f){x, y, z});
}

// TODO(ambrus): not yet tested.
static inline ac_v2f ac_llint_3f(ac_v3f p0, ac_v3f v0, ac_v3f p1, ac_v3f v1) {
  // Solve the system of equations: s0 * v0 + p0 = s1 * v1 + p1
  //                 [s0]
  //  => [v0, -v1] * [s1] = [p1 - p0]
  //  =>     A     *   x  =     b0
  // Least squares solution is: x = (A_T.A)^(-1).A_T.b0
  //
  //  b0 = [p1 - p0]
  //
  //           [ v0_T ]        [ v0 . b0 ]
  //  A_T.b0 = [-v1_T ] . b0 = [ v1 . b0 ]
  //
  //          [a, b]   [ v0_T ]
  //  A_T.A = [c, d] = [-v1_T ] . [v0, -v1]
  //
  //                            [ d, -b]
  //  (A_T.A)^(-1) = 1/(ad - bc)[-c,  a]

  // Right-hand side of equation.
  const ac_v3f b0 = ac_v3f_sub(p1, p0);

  // Elements of A_T.b0
  const float ATb0x = ac_v3f_dot(v0, b0);
  const float ATb0y = ac_v3f_dot(v1, b0);
  const ac_v2f ATb0 = {ATb0x, ATb0y};

  // Elements of A_T.A
  const float a = ac_v3f_len2(v0);
  const float b = -ac_v3f_dot(v0, v1);
  const float c = b;
  const float d = ac_v3f_len2(v1);

  // Determinant of A_T.A
  const float det = 1.f / (a * d - b * c);

  // Elements of (A_T.A)^(-1). NOTE: det omitted here, multiplied below.
  const ac_v2f ATA_inv_row0 = {d, -b};
  const ac_v2f ATA_inv_row1 = {-c, a};

  // Elements of s
  const float s0 = det * ac_v2f_dot(ATA_inv_row0, ATb0);
  const float s1 = det * ac_v2f_dot(ATA_inv_row1, ATb0);
  return (ac_v2f){s0, s1};
}

// TODO(ambrus): not yet tested.
static inline ac_v3f ac_lldist2_3f(ac_v3f p0, ac_v3f v0, ac_v3f p1, ac_v3f v1) {
  const ac_v2f s = ac_llint_3f(p0, v0, p1, v1);
  const ac_v3f q0 = ac_v3f_add(p0, ac_v3f_scale(v0, s.x));
  const ac_v3f q1 = ac_v3f_add(p1, ac_v3f_scale(v1, s.y));
  const float d = ac_v3f_len2(ac_v3f_sub(q0, q1));
  return (ac_v3f){s.x, s.y, d};
}

#endif  // AC_VECH_H
