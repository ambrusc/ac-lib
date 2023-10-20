#ifndef AC_TEST_TEST_H_
#define AC_TEST_TEST_H_

#include "ac_alloc.h"
#include "ac_test.h"

//------------------------------------------------------------------------------
// Test Case Execution and Nesting
//------------------------------------------------------------------------------

static inline void no_begin_fails(ac_test_state* s) {
  (void)s;
  // Should fail without ac_test_begin(s);
}

static inline void single_case_ok(ac_test_state* s) {
  ac_test_begin(s);
  // Nothing to do but succeed.
}

static inline void single_case_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_fail("Single test case.");
}

static inline void single_case_3_fails(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_fail("Failure 1.");
  ac_test_fail("Failure 2.");
  ac_test_fail("Failure 3.");
}

static inline void single_case_return_1st_fail(ac_test_state* s) {
  ac_test_begin(s);
  if (!ac_test_fail("Failure 1.")) return;
  if (!ac_test_fail("Failure 2.")) return;
  if (!ac_test_fail("Failure 3.")) return;
}

static inline void nested_case_succees(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_run(single_case_ok);
}

static inline void nested_case_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_run(single_case_ok);
}

static inline void nested_case_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_run(single_case_fail);
}

static inline void nested_case_3_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_run(single_case_ok);
  ac_test_run(single_case_ok);
  ac_test_run(single_case_ok);
}

static inline void nested_case_2_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_run(single_case_fail);
  ac_test_run(single_case_ok);
  ac_test_run(single_case_fail);
}

static inline void nested_case_parent_fail_before(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_fail("Before case.");
  ac_test_run(single_case_fail);
}

static inline void nested_case_parent_fail_after(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_run(single_case_fail);
  ac_test_fail("After case.");
}

static inline void expect_true_doesnt_return(ac_test_state* s) {
  ac_test_begin(s);
  if (!ac_test_expect(true, "Not a failure. Should run next case.")) return;
  ac_test_run(single_case_ok);
}

static inline void expect_false_returns(ac_test_state* s) {
  ac_test_begin(s);
  if (!ac_test_expect(false, "Failure. Don't run next case.")) return;
  ac_test_run(single_case_fail);
}

static inline void expect_runs_ok_doesnt_return(ac_test_state* s) {
  ac_test_begin(s);
  if (!ac_test_run(single_case_ok)) return;
  ac_test_run(single_case_ok);  // Must run.
}

static inline void expect_runs_failure_returns(ac_test_state* s) {
  ac_test_begin(s);
  if (!ac_test_run(single_case_fail)) return;
  ac_test_run(single_case_fail);  // Mustn't run.
}

//------------------------------------------------------------------------------
// Signed Integer Assertions
//------------------------------------------------------------------------------

static inline void eqi_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_eqi(3, 3);
}

static inline void eqi_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_eqi(3, 4);
}

static inline void nei_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_nei(3, 4);
}

static inline void nei_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_nei(3, 3);
}

static inline void gti_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gti(4, 3);
}

static inline void gti_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gti(3, 3);
}

static inline void lti_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_lti(3, 4);
}

static inline void lti_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_lti(3, 3);
}

static inline void gei_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gei(3, 3);
}

static inline void gei_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gei(3, 4);
}

static inline void lei_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_lei(3, 3);
}

static inline void lei_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_lei(4, 3);
}

//------------------------------------------------------------------------------
// Unsigned Integer Assertions
//------------------------------------------------------------------------------

static inline void equ_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_equ(3, 3);
}

static inline void equ_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_equ(3, 4);
}

static inline void neu_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_neu(3, 4);
}

static inline void neu_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_neu(3, 3);
}

static inline void gtu_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gtu(4, 3);
}

static inline void gtu_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gtu(3, 3);
}

static inline void ltu_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_ltu(3, 4);
}

static inline void ltu_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_ltu(3, 3);
}

static inline void geu_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_geu(3, 3);
}

static inline void geu_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_geu(3, 4);
}

static inline void leu_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_leu(3, 3);
}

static inline void leu_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_leu(4, 3);
}

//------------------------------------------------------------------------------
// Floating Point Assertions
//------------------------------------------------------------------------------

static inline void eqf_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_eqf(3.1, 3.1);
}

static inline void eqf_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_eqf(3.1, 3.2);
}

static inline void nef_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_nef(3.1, 3.2);
}

static inline void nef_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_nef(3.1, 3.1);
}

static inline void gtf_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gtf(4.1, 3.1);
}

static inline void gtf_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gtf(3.1, 3.1);
}

static inline void ltf_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_ltf(3.1, 4.1);
}

static inline void ltf_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_ltf(3.1, 3.1);
}

static inline void gef_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gef(3.1, 3.1);
}

static inline void gef_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_gef(3.1, 4.1);
}

static inline void lef_ok(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_lef(3.1, 3.1);
}

static inline void lef_fail(ac_test_state* s) {
  ac_test_begin(s);
  ac_test_lef(4.1, 3.1);
}

//------------------------------------------------------------------------------
// Harness
//------------------------------------------------------------------------------

// Points to a test case.
typedef void (*ac_test_test_case_ptr)(ac_test_state*);

// Runs a test case in a fresh test environment. This is useful for testing
// functions that are supposed to fail, and counting assertions/expectations.
static inline void ac_test_test_run_case_(ac_test_state* s, ac_str* out_str,
                                          size_t expected_cases,
                                          size_t expected_failures,
                                          const char* name,
                                          ac_test_test_case_ptr func) {
  ac_test_begin_with_params(s);
  ac_test_log_param_cstr(name);
  ac_test_end_params();

  size_t actual_cases = 0;
  size_t actual_failures = 0;

#ifdef _POSIX_C_SOURCE
  FILE* out = fmemopen(out_str->data, out_str->cap, "w");
#else
  FILE* out = tmpfile();
#endif

  if (!ac_test_expect(out != NULL, "Failed to fmemopen in test harness.")) {
    return;
  }

  {
    ac_test_init((ac_test_opts){out});
    ac_test_run(func);
    actual_cases = ac_test_state()->cases_run;
    actual_failures = ac_test_state()->failures;
    ac_test_done();
  }

  const size_t file_size = ftell(out);
  const size_t size = out_str->cap < file_size ? out_str->cap : file_size;
#ifndef _POSIX_C_SOURCE
  rewind(out);
  fread(out_str->data, /*size=*/1, size, out);
#endif
  out_str->len = size;
  fclose(out);

  ac_test_equ(actual_cases, expected_cases);
  ac_test_equ(actual_failures, expected_failures);
}

//------------------------------------------------------------------------------
// Test calls
//------------------------------------------------------------------------------

// Entry point for all the rest of the tests.
static inline void ac_test_test(ac_test_state* s) {
  ac_test_begin(s);

  // Stores the printed output of each test case. Ignored for now since we can
  // check assertion/expectation counts.
  ac_str out_str = ac_str_init(ac_mallocator2());
  ac_lista_realloc(&out_str, 4 * 1024 * 1024);

#define run_case_(cases, failures, test_func)                           \
  ac_test_run(ac_test_test_run_case_, &out_str, cases, failures, \
                     #test_func, &test_func)

  // Basics, nesting.
  run_case_(/*cases=*/1, /*failures=*/1, no_begin_fails);
  run_case_(/*cases=*/1, /*failures=*/0, single_case_ok);
  run_case_(/*cases=*/1, /*failures=*/1, single_case_fail);
  run_case_(/*cases=*/1, /*failures=*/3, single_case_3_fails);
  run_case_(/*cases=*/1, /*failures=*/1, single_case_return_1st_fail);
  run_case_(/*cases=*/2, /*failures=*/0, nested_case_ok);
  run_case_(/*cases=*/2, /*failures=*/1, nested_case_fail);
  run_case_(/*cases=*/4, /*failures=*/0, nested_case_3_ok);
  run_case_(/*cases=*/4, /*failures=*/2, nested_case_2_fail);
  run_case_(/*cases=*/2, /*failures=*/2, nested_case_parent_fail_before);
  run_case_(/*cases=*/2, /*failures=*/2, nested_case_parent_fail_after);
  run_case_(/*cases=*/2, /*failures=*/0, expect_true_doesnt_return);
  run_case_(/*cases=*/1, /*failures=*/1, expect_false_returns);
  run_case_(/*cases=*/3, /*failures=*/0, expect_runs_ok_doesnt_return);
  run_case_(/*cases=*/2, /*failures=*/1, expect_runs_failure_returns);

  // Signed int inequalities.
  run_case_(/*cases=*/1, /*failures=*/0, eqi_ok);
  run_case_(/*cases=*/1, /*failures=*/1, eqi_fail);
  run_case_(/*cases=*/1, /*failures=*/0, nei_ok);
  run_case_(/*cases=*/1, /*failures=*/1, nei_fail);
  run_case_(/*cases=*/1, /*failures=*/0, gti_ok);
  run_case_(/*cases=*/1, /*failures=*/1, gti_fail);
  run_case_(/*cases=*/1, /*failures=*/0, lti_ok);
  run_case_(/*cases=*/1, /*failures=*/1, lti_fail);
  run_case_(/*cases=*/1, /*failures=*/0, gei_ok);
  run_case_(/*cases=*/1, /*failures=*/1, gei_fail);
  run_case_(/*cases=*/1, /*failures=*/0, lei_ok);
  run_case_(/*cases=*/1, /*failures=*/1, lei_fail);

  // Unsigned int inequalities.
  run_case_(/*cases=*/1, /*failures=*/0, equ_ok);
  run_case_(/*cases=*/1, /*failures=*/1, equ_fail);
  run_case_(/*cases=*/1, /*failures=*/0, neu_ok);
  run_case_(/*cases=*/1, /*failures=*/1, neu_fail);
  run_case_(/*cases=*/1, /*failures=*/0, gtu_ok);
  run_case_(/*cases=*/1, /*failures=*/1, gtu_fail);
  run_case_(/*cases=*/1, /*failures=*/0, ltu_ok);
  run_case_(/*cases=*/1, /*failures=*/1, ltu_fail);
  run_case_(/*cases=*/1, /*failures=*/0, geu_ok);
  run_case_(/*cases=*/1, /*failures=*/1, geu_fail);
  run_case_(/*cases=*/1, /*failures=*/0, leu_ok);
  run_case_(/*cases=*/1, /*failures=*/1, leu_fail);

  // Floating point inequalities.
  run_case_(/*cases=*/1, /*failures=*/0, eqf_ok);
  run_case_(/*cases=*/1, /*failures=*/1, eqf_fail);
  run_case_(/*cases=*/1, /*failures=*/0, nef_ok);
  run_case_(/*cases=*/1, /*failures=*/1, nef_fail);
  run_case_(/*cases=*/1, /*failures=*/0, gtf_ok);
  run_case_(/*cases=*/1, /*failures=*/1, gtf_fail);
  run_case_(/*cases=*/1, /*failures=*/0, ltf_ok);
  run_case_(/*cases=*/1, /*failures=*/1, ltf_fail);
  run_case_(/*cases=*/1, /*failures=*/0, gef_ok);
  run_case_(/*cases=*/1, /*failures=*/1, gef_fail);
  run_case_(/*cases=*/1, /*failures=*/0, lef_ok);
  run_case_(/*cases=*/1, /*failures=*/1, lef_fail);

#undef run_case_

  ac_str_free(&out_str);
}

#endif  // AC_TEST_TEST_H_
