#ifndef AC_TIME_TEST_H_
#define AC_TIME_TEST_H_

#include "ac_alloc.h"
#include "ac_math.h"
#include "ac_test.h"

#define AC_TIME_IMPL
#include "ac_time.h"

static inline void test_time(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gtu(ac_time_now().ticks, 0);
  ac_test_gtu(ac_time_freq(), 0);
  ac_test_eqi(ac_time_diff((ac_time){3}, (ac_time){1}).dticks, 2);
}

static inline void test_cputime(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gtu(ac_cputime_now().cpu_ticks, 0);
  ac_test_gtu(ac_cputime_freq(), 0);
  ac_test_eqi(ac_cputime_diff((ac_cputime){3}, (ac_cputime){1}).cpu_dticks, 2);
}

static inline void test_cputime_measure_freq(ac_test_state* s) {
  ac_test_begin(s);
  // NOTE: on some platforms the Cpu-time frequency cannot by queried directly
  // and is therefore measured. On those platforms, the values should be exactly
  // equal. On other platforms, we want to make sure that the values are within
  // a reasonable threshold.
  const uint64_t freq_measured = ac_cputime_measure_freq();
  const uint64_t freq_queried = ac_cputime_freq();
  const int64_t thresh = 0.01 * ac_max(freq_measured, freq_queried) + 0.5;
  ac_test_gei(freq_measured, freq_queried - thresh);
  ac_test_lei(freq_measured, freq_queried + thresh);
}

static inline void test_sleep_ms(ac_test_state* s) {
  ac_test_begin(s);

  bool success = false;
  for (size_t i = 0; i < 10; ++i) {
    const ac_time t0 = ac_time_now();
    const ac_cputime c0 = ac_cputime_now();

    const size_t target_ms = 50;
    ac_sleep_ms(target_ms);

    const ac_dtime dt = ac_time_diff(ac_time_now(), t0);
    const ac_dcputime dc = ac_cputime_diff(ac_cputime_now(), c0);
    const int delta_ms = 1000 * dt.dticks / ac_time_freq();
    const int cpu_delta_ms = 1000 * dc.cpu_dticks / ac_cputime_freq();

    const int max_ms = 3 * target_ms / 2;
    const int min_ms = target_ms - 2;

    if (min_ms <= delta_ms && delta_ms <= max_ms && min_ms <= cpu_delta_ms &&
        cpu_delta_ms <= max_ms) {
      success = true;
      break;
    }
  }

  ac_test_expect(success);
}

static inline void test_time_ms_since_1970_round_trip(ac_test_state* s) {
  ac_test_begin(s);

  const uint64_t target = 123456;
  const ac_time t = ac_time_from_ms_since_1970(target);
  ac_test_gtu(t.ticks, 0);

  const uint64_t ms = ac_time_ms_since_1970(t);
  ac_test_gei(ms, target - 1);
  ac_test_lei(ms, target + 1);
}

static inline void test_time_sec_since_2000_round_trip(ac_test_state* s) {
  ac_test_begin(s);

  const double target = 123456;
  const ac_time t = ac_time_from_sec_since_2000(target);
  ac_test_gtu(t.ticks, 0);

  const double secs = ac_time_sec_since_2000(t);
  ac_test_gef(secs, target - 1e-3);
  ac_test_lef(secs, target + 1e-3);
}

static inline void test_time_conversions_1970_2000(ac_test_state* s) {
  ac_test_begin(s);

  // Julian date of 2000-noon is 2451545.0
  // Julian date of 1970-midnight is 2440587.5
  // Their difference is 10957.5 days.
  const double secs_from_1970_to_2000 = 10957.5 * 24 * 60 * 60;
  const uint64_t ms_from_1970_to_2000 = secs_from_1970_to_2000 * 1000 + 0.5;
  printf("%" PRIu64 "\n", ms_from_1970_to_2000);
  const ac_time t = ac_time_now();

  // Same time point, two ways to get there.
  const uint64_t a = ac_time_ms_since_1970(t) - ms_from_1970_to_2000;
  const uint64_t b = 1000 * ac_time_sec_since_2000(t) + 0.5;
  ac_test_lei(a - b, 1);
}

// Entry point for all the rest of the tests.
static inline void ac_time_test(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_run(test_time);
  ac_test_run(test_cputime);
  ac_test_run(test_cputime_measure_freq);
  ac_test_run(test_sleep_ms);
  ac_test_run(test_time_ms_since_1970_round_trip);
  ac_test_run(test_time_sec_since_2000_round_trip);
  ac_test_run(test_time_conversions_1970_2000);
}

#endif  // AC_TIME_TEST_H_
