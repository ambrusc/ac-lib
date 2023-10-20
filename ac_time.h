#ifndef AC_TIME_H_
#define AC_TIME_H_

#include <stdbool.h>
#include <stdint.h>

// Sleep for at least 'ms' milliseconds.
static inline void ac_sleep_ms(uint64_t ms);

// General purpose time struct.
typedef struct ac_time {
  uint64_t ticks;
} ac_time;

// General purpose time delta.
typedef struct ac_dtime {
  int64_t dticks;
} ac_dtime;

// High frequency CPU counter timestamp for benchmarking.
// Unsuitable for date/time.
typedef struct ac_cputime {
  uint64_t cpu_ticks;
} ac_cputime;

// High frequency CPU counter time delta for benchmarking.
// Unsuitable for date/time.
typedef struct ac_dcputime {
  int64_t cpu_dticks;
} ac_dcputime;

// Current time using the OS/Platform clocks.
static inline ac_time ac_time_now();
static inline ac_cputime ac_cputime_now();

// Frequency information for OS and CPU timers (Hz).
// NOTE: CPU timer frequency May be approximate, measured on first call.
// Subsequent calls return the initially measured value.
static inline uint64_t ac_time_freq();
#if defined(__x86_64__)
uint64_t ac_cputime_freq();
#else
static inline uint64_t ac_cputime_freq();
#endif

// Measures CPU timer frequency using the OS timer (Hz).
static inline uint64_t ac_cputime_measure_freq();

// Time diffs (a - b).
static inline ac_dtime ac_time_diff(ac_time a, ac_time b);
static inline ac_dcputime ac_cputime_diff(ac_cputime a, ac_cputime b);

// Unix Timestamp: Milliseconds since UTC 1970 Jan 1 midnight.
static inline int64_t ac_time_ms_since_1970(ac_time a);
static inline ac_time ac_time_from_ms_since_1970(uint64_t a);
static inline ac_time ac_time_1970_midnight();

// Julian Date UT1: Seconds since UTC 2000 Jan 1 NOON.
static inline double ac_time_sec_since_2000(ac_time a);
static inline ac_time ac_time_from_sec_since_2000(double a);
static inline ac_time ac_time_2000_noon();

//------------------------------------------------------------------------------
// Static Inline Implementation
//------------------------------------------------------------------------------

#if defined(__linux__) || defined(__APPLE__)  //---------- LINUX/MAC -----------

#include <time.h>

static inline void ac_sleep_ms(uint64_t ms) {
  const uint64_t sec = ms / 1000;
  const uint64_t nanos = (ms - sec * 1000) * 1000000;
  struct timespec request = {.tv_sec = sec, .tv_nsec = nanos};
  struct timespec remain = {};
  while (nanosleep(&request, &remain)) {
    request = remain;
  }
}

#include <sys/time.h>

static inline ac_time ac_time_now() {
  // Epoch: UTC 1970 Jan 1 midnight.
  struct timeval value;
  gettimeofday(&value, 0);
  return (ac_time){((uint64_t)value.tv_sec) * 1000000 + value.tv_usec};
}

static inline uint64_t ac_time_freq() { return 1000000; }  // 1 microsec / tick.

static inline ac_time ac_time_1970_midnight() { return (ac_time){0}; };
static inline ac_time ac_time_2000_noon() {
  // Julian date of 2000-noon is 2451545.0
  // Julian date of 1970-midnight is 2440587.5
  // Their difference is 10957.5 days.
  // 10957.5 * (24 * 60 * 60 * 1000 * 1000) = 946728000000000 microsec.
  return (ac_time){946728000000000};
};

#elif defined(_WIN32)  //---------------------- WIN32 --------------------------

#include <synchapi.h>

static inline void ac_sleep_ms(uint64_t ms) {
  SleepEx(ms, /*bAlertable=*/false);
}

#include <sysinfoapi.h>

static inline ac_unixt ac_time_now() {
  // Epoch: UTC 1601 Jan 1 midnight.
  FILETIME time;
  GetSystemTimePreciseAsFileTime(&time);
  ULARGE_INTEGER ticks;
  ticks.LowPart = time.dwLowDateTime;
  ticks.HighPart = time.dwHighDateTime;
  return (ac_time){ticks.QuadPart};
}

static inline uint64_t ac_time_freq() { return 10000000; }  // 100 ns / tick.

static inline ac_time ac_time_1970_midnight() {
  // https://stackoverflow.com/questions/20370920/convert-current-time-from-windows-to-unix-timestamp-in-c-or-c
  return (ac_time){116444736000000000};
};
static inline ac_time ac_time_2000_noon() {
  // Julian date of 2000-noon is 2451545.0
  // Julian date of 1970-midnight is 2440587.5
  // Their difference is 10957.5 days.
  // 10957.5 * (24 * 60 * 60 * 1000 * 1000 * 10) = 9467280000000000 ticks.
  // 116444736000000000 + 946728000000000 = 117391464000000000.
  return (ac_time){117391464000000000};
};

#endif  //----------------------------------------------------------------------

#if defined(__x86_64__)  //-------------------- X86-64 -------------------------

#include <x86intrin.h>

static inline ac_cputime ac_cputime_now() { return (ac_cputime){__rdtsc()}; }

#elif defined(__aarch64__)  //------------------- ARM --------------------------

static inline ac_cputime ac_cputime_now() {
  uint64_t ticks;
  __asm__ volatile("mrs %0, cntvct_el0" : "=r"(ticks));
  return (ac_cputime){ticks};
}

static inline uint64_t ac_cputime_freq() {
  uint64_t freq;
  __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));
  return freq;
}

#endif  //----------------------------------------------------------------------

static inline uint64_t ac_cputime_measure_freq() {
  const int64_t wait_ticks = ac_time_freq() / 200;
  double smallest_ratio = 1e15;
  for (size_t i = 0; i < 10; ++i) {
    const ac_time t0 = ac_time_now();
    const ac_cputime cpu_t0 = ac_cputime_now();
    ac_dtime dt = {};
    while (dt.dticks < wait_ticks) {
      dt = ac_time_diff(ac_time_now(), t0);
    }
    const ac_dcputime dt_cpu = ac_cputime_diff(ac_cputime_now(), cpu_t0);
    const double ratio = 1.0 * dt_cpu.cpu_dticks / dt.dticks;
    if (ratio < smallest_ratio) smallest_ratio = ratio;
  }
  return smallest_ratio * ac_time_freq();
}

static inline ac_dtime ac_time_diff(ac_time a, ac_time b) {
  return (ac_dtime){a.ticks - b.ticks};
}

static inline ac_dcputime ac_cputime_diff(ac_cputime a, ac_cputime b) {
  return (ac_dcputime){a.cpu_ticks - b.cpu_ticks};
}

static inline int64_t ac_time_ms_since_1970(ac_time a) {
  const uint64_t ticks_per_ms = ac_time_freq() / 1000;
  const ac_dtime d = ac_time_diff(a, ac_time_1970_midnight());
  return d.dticks / ticks_per_ms;
}

static inline ac_time ac_time_from_ms_since_1970(uint64_t a) {
  const uint64_t ticks_per_ms = ac_time_freq() / 1000;
  return (ac_time){a * ticks_per_ms + ac_time_1970_midnight().ticks};
}

static inline double ac_time_sec_since_2000(ac_time a) {
  const double sec_per_tick = 1.f / ac_time_freq();
  const ac_dtime d = ac_time_diff(a, ac_time_2000_noon());
  return d.dticks * sec_per_tick;
}

static inline ac_time ac_time_from_sec_since_2000(double a) {
  const double ticks_per_sec = ac_time_freq();
  const int64_t ticks = a * ticks_per_sec + 0.5f;
  return (ac_time){ticks + ac_time_2000_noon().ticks};
}

// #if _WIN32

// #include <intrin.h>
// #include <windows.h>

// uint64_t timer_os_freq() {
//   ULARGE_INTEGER freq;
//   QueryPerformanceFrequency(&freq);
//   return freq.QuadPart;
// }

// uint64_t timer_os_ticks() {
//   ULARGE_INTEGER value;
//   QueryPerformanceCounter(&value);
//   return value.QuadPart;
// }

// #else

#endif  // AC_TIME_H_

//------------------------------------------------------------------------------
// Non-Static Implementation
//------------------------------------------------------------------------------

#if defined(AC_TIME_IMPL)

#if defined(__x86_64__)  //-------------------- X86-64--------------------------

uint64_t ac_cputime_freq() {
  static uint64_t freq = 0;
  if (!freq) freq = ac_cputime_measure_freq();
  return freq;
}

#endif  //----------------------------------------------------------------------

#endif  // AC_TIME_IMPL
