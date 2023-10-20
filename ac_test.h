//
// Example API Usage:
//
//   int main(...) {
//     ac_test_init(...);
//     ac_test_run(my_test_case, 123);
//     ac_test_run(my_test_case, 456);
//     ac_test_run(...);
//     ac_test_run(...);
//     return ac_test_done() ? 0 : 1;
//   }
//
//   void my_test_case(ac_test_state* s, int my_param) {
//     ac_test_begin_with_params(s);    // Tells the test lib the current state.
//     ac_test_parami(my_param);        // Logs 'my_param' value to test output.
//     ac_test_end_params();
//
//     // Fails the test case if my_param != 123, then continues execution.
//     ac_test_eqi(my_param, 123);
//
//     // Returns immediately if my_param != 456.
//     if (!ac_test_eqi(my_param, 456)) return;
//
//     // Print user-specified output.
//     ac_test_print("\n%s %s", "SOME", "OPTIONAL OUTPUT");
//     ac_test_print_str(my_str);
//   }
//
//
//------------------------------------------------------------------------------
// Example Output:
//
//   my_test_case ( my_param: 123 )
//
//   Failed: my_param == 456
//     my_param: 123
//     456: 456
//
//   example_usage.c:19  in  my_test_case
//   example_usage.c:3  in  main
//
//   my_test_case ( my_param: 123 ) ......................................FAILED
//   my_test_case ( my_param: 456 )
//
//   Failed: my_param == 456
//     my_param: 123
//     456: 456
//
//   example_usage.c:19  in  my_test_case
//   example_usage.c:3  in  main
//
//   SOME OPTIONAL OUTPUT
//   SOME OTHER OPTIONAL OUTPUT
//
//   my_test_case ( my_param: 456 ) ......................................FAILED
//   my_second_test_case ...ok
//   my_third_test_case ...ok
//
//   (cases_run:4 failures:2) ......................................TESTS FAILED
//
//
//------------------------------------------------------------------------------
// Operator shorthand:
//   eq  ==
//   ne  !=
//   lt  <
//   le  <=
//   gt  >
//   ge  >=
//
// Suffix shorthand:
//   i   integer
//   u   unsigned integer
//   f   floating point (usu. double)
//
//

#ifndef AC_TEST_H_
#define AC_TEST_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "ac_alloc.h"
#include "ac_str.h"

//------------------------------------------------------------------------------
// Source Location.
//------------------------------------------------------------------------------

// Printable source file location.
typedef struct ac_test_source_loc {
  const char* file;
  int line;
  const char* func;
} ac_test_source_loc;

// Human-readable verison of a source location.
static inline void ac_test_source_loc_to_str(ac_str* str,
                                             const ac_test_source_loc* a) {
  ac_to_str(str, "%s:%d  in  %s", a->file, a->line, a->func);
}

// Current source file location.
#define ac_test_source_loc_here() \
  (ac_test_source_loc) { .file = __FILE__, .line = __LINE__, .func = __func__ }

//------------------------------------------------------------------------------
// Test Case Call Stack.
//------------------------------------------------------------------------------

// One node in the test case call stack.
typedef struct ac_test_stack_node {
  // Source location of the call site.
  ac_test_source_loc caller;

  // Test function name.
  const char* func;

  // First letter of this test function's name/params on the name stack.
  size_t name_begin;

  // Used to determine if output was printed by the test library after this
  // funciton's name. If so, the name is printed again before the status is.
  size_t print_count_after_name;

  // Counts of failures.
  size_t failures;
} ac_test_stack_node;
ac_lista_define_type(ac_test_stack_node);

//------------------------------------------------------------------------------
// Static Assertions to ensure correct API call ordering.
//------------------------------------------------------------------------------

// Default values for enums used to statically determine correct API usage.
// These values are shadowed in the local scope by certain calls
enum {
  ac_test_begin_or_init_was_called_ = 0,
  ac_test_begin_with_params_was_called_ = 0,
  ac_test_end_params_was_called_ = 0
};

// Compile-time assert that begin/init was called in the current test.
#define ac_test_static_assert_begin_or_init_was_called_()                    \
  _Static_assert(                                                            \
      ac_test_begin_or_init_was_called_,                                     \
      "Before using any other test APIs, test state must be initialized on " \
      "the stack. Call 'ac_test_init(...)' once at toplevel, and "           \
      "'ac_test_begin(...)' once inside each test function.");

// Compile-time assert that begin_with_params was called.
#define ac_test_static_assert_begin_with_params_was_called_()                 \
  _Static_assert(                                                             \
      ac_test_begin_with_params_was_called_,                                  \
      "Use 'ac_test_begin_with_params(...)' instead of 'ac_test_begin(...)' " \
      "for cases which log their parameters.");

// Compile-time assert that begin_with_params WASN'T called.
#define ac_test_static_assert_end_params_not_called_()    \
  _Static_assert(                                         \
      ac_test_end_params_was_called_ == 0,                \
      "Calls to 'ac_test_param...(...)' must be between " \
      "'ac_test_begin_with_params(...)' and 'ac_test_end_params(...)'.");

// Compile-time assert that begin/end-params were both called or neither called.
#define ac_test_static_assert_params_complete_()                               \
  _Static_assert(                                                              \
      ac_test_begin_with_params_was_called_ == ac_test_end_params_was_called_, \
      "'ac_test_end_params()' must be called before using non-parameter test " \
      "APIs.");

//------------------------------------------------------------------------------
// Test State.
//------------------------------------------------------------------------------

// User-specified options for tests.
typedef struct ac_test_opts {
  // All test output is written to this file. If NULL, stdout is used.
  FILE* output;
  // Allocator for internal memory. Used sparingly. If default, malloc is used.
  ac_allocator2 alloc;
  // Right-alignment of the FAILED message. If default, 80 is used.
  size_t failed_msg_column;
} ac_test_opts;

// State for a group of test cases.
typedef struct ac_test_state {
  size_t failed_msg_column;
  FILE* out;
  ac_str scratch;
  ac_str names;
  ac_lista(ac_test_stack_node) stack;

  // Flag: next call must (or mustn't) be 'ac_test_init/begin'.
  bool expect_test_begin;
  // Location of the current failure (if any).
  ac_test_source_loc failure_loc;
  // Incrementing counter of prints to 'out'.
  size_t print_count;

  // Total counts for all test cases.
  size_t cases_run;
  size_t failures;
} ac_test_state;

// Prints to state->out with fprintf, incrementing test_state->print_count.
#define ac_test_print_(state_ptr, ...)      \
  ({                                        \
    ++((state_ptr)->print_count);           \
    fprintf((state_ptr)->out, __VA_ARGS__); \
  })

// Writes the ac_str to state->out, incrementing test_state->print_count.
#define ac_test_print_str_(state_ptr, str)                   \
  ({                                                         \
    ++((state_ptr)->print_count);                            \
    fwrite(str.data, /*size=*/1, str.len, (state_ptr)->out); \
  })

// Frees memory for a group of test cases.
static inline void ac_test_state_free_(ac_test_state* s) {
  ac_lista_free(&s->stack);
  s->stack = (ac_lista(ac_test_stack_node)){};
  ac_lista_free(&s->names);
  s->names = (ac_str){};
  ac_lista_free(&s->scratch);
  s->scratch = (ac_str){};
}

// Allocates memory for a group of test cases.
static inline void ac_test_state_init_(ac_test_state* s, ac_test_opts opts) {
  // Clear any memory in case this function is called multiple times.
  ac_test_state_free_(s);
  // Failed message column defaults to 80.
  s->failed_msg_column = opts.failed_msg_column ? opts.failed_msg_column : 80;
  // Output defaults to stdout.
  s->out = opts.output ? opts.output : stdout;
  // Set the allocators to the user-specified one.
  // If ops.alloc is empty, lista defaults to malloc internally.
  s->stack.alloc = opts.alloc;
  s->names.alloc = opts.alloc;
  s->scratch.alloc = opts.alloc;
  // Reserve memory.
  ac_lista_realloc(&s->stack, 128);
  ac_lista_realloc(&s->names, 512);
  ac_lista_realloc(&s->scratch, 16 * 1024);
  // Test names should always be printed on a new line.
  ac_to_str(&s->names, "\n");
}

// Stringifies the current call stack.
static inline void ac_test_stack_to_str_(ac_str* str, const ac_test_state* s) {
  ac_to_str(str, "  ");
  ac_test_source_loc_to_str(str, &s->failure_loc);
  ac_to_str(str, "\n");

  for (size_t i = s->stack.len - 1; i < s->stack.len; --i) {
    ac_to_str(str, "  ");
    ac_test_source_loc_to_str(str, &s->stack.data[i].caller);
    ac_to_str(str, "\n");
  }
}

// Prints final stats e.g. after all test cases are done.
static inline void ac_test_print_stats_(ac_test_state* s) {
  // Stringify the stats.
  fprintf(s->out, "\n\n");
  s->scratch.len = 0;
  ac_to_str(&s->scratch, "(cases_run:%zu ", s->cases_run);
  ac_to_str(&s->scratch, "failures:%zu) ", s->failures);
  if (s->failures) {
    if (s->scratch.len + 15 < s->failed_msg_column) {
      ac_str_repeat_char(&s->scratch, '.',
                         s->failed_msg_column - s->scratch.len - 15);
    }
    ac_to_str(&s->scratch, "...TESTS FAILED\n\n");
  } else {
    ac_to_str(&s->scratch, "...tests ok\n\n");
  }

  // Print them.
  ac_test_print_str_(s, s->scratch);
  fflush(s->out);
}

// Called when an assertion/expectation fails. Updates state and failure counts.
static inline void ac_test_fail_(ac_test_state* s, ac_test_source_loc loc) {
  // Global state.
  s->failure_loc = loc;
  ++s->failures;

  // Local state.
  if (s->stack.len) {
    ac_test_stack_node* n = s->stack.data + s->stack.len - 1;
    ++n->failures;
  }
}

// Updates state and call stack before a test case is called.
static inline void ac_test_before_call_(ac_test_state* s,
                                        ac_test_source_loc loc) {
  // Push the calling location onto the stack.
  *(ac_lista_next_ex(&s->stack)) =
      (ac_test_stack_node){.caller = loc, .name_begin = s->names.len};

  // Bookkeeping.
  s->expect_test_begin = true;
  ++s->cases_run;
}

// Called when a test case begins executing. Updates state and prints info.
static inline void ac_test_begin_call_(ac_test_state* s,
                                       ac_test_source_loc loc) {
  // Store call state at the top of the stack.
  ac_test_stack_node* n = s->stack.data + s->stack.len - 1;
  n->name_begin = s->names.len;
  n->func = loc.func;

  // Stringify the function's name.
  if (s->stack.len > 1) ac_to_str(&s->names, " ");
  ac_to_str(&s->names, "%s", loc.func);

  // Then print the name stack.
  ac_test_print_str_(s, s->names);
  n->print_count_after_name = s->print_count;
}

// Updates state and call stack after a test case has been called.
// Returns 'true' if the caller ran without logging any test failures.
static inline bool ac_test_after_call_(ac_test_state* s) {
  // TODO(ambrus): this is a bug or misuse.
  if (!s->stack.len) return false;

  const ac_test_stack_node* n = s->stack.data + s->stack.len - 1;
  const bool test_case_ok = !(n->failures);

  // Print the name again before the status if other output was interleaved.
  if (s->print_count != n->print_count_after_name) {
    ac_test_print_str_(s, s->names);
  }

  // Print the status and record failures.
  if (test_case_ok) {
    ac_test_print_(s, " ...ok");
  } else {
    ac_test_print_(s, " ");
    if (s->names.len + 9 < s->failed_msg_column) {
      s->scratch.len = 0;
      ac_str_repeat_char(&s->scratch, '.',
                         s->failed_msg_column - s->names.len - 9);
      ac_test_print_str_(s, s->scratch);
    }
    ac_test_print_(s, "...FAILED");

    // If the callee failed, register a failure in the caller too.
    // However, DON'T increment the total failure count every time we pop up the
    // stack, because it inflates the number of failures based on the failing
    // code's position in the call stack, which doesn't seem to have a clear
    // benefit.
    if (s->stack.len > 2) {
      ac_test_stack_node* caller = s->stack.data + s->stack.len - 2;
      ++caller->failures;
    }
  }
  fflush(s->out);

  // Pop the stack.
  s->names.len = n->name_begin;
  --s->stack.len;

  return test_case_ok;
}

//------------------------------------------------------------------------------
// Implementation Details.
//------------------------------------------------------------------------------

// Fails the test case but doesn't return. Always evalutes to 'false'.
#define ac_test_fail_(...)                                   \
  ({                                                         \
    ac_test_static_assert_begin_or_init_was_called_();       \
    ac_test_static_assert_params_complete_();                \
    /* Set the test failed state the user message. */        \
    ac_test_fail_(ac_test_s_, ac_test_source_loc_here());    \
    /* Stringify the user message. */                        \
    ac_test_s_->scratch.len = 0;                             \
    ac_to_str(&ac_test_s_->scratch, "\n\n");                 \
    ac_to_str(&ac_test_s_->scratch, __VA_ARGS__);            \
    ac_to_str(&ac_test_s_->scratch, "\n\n");                 \
    /* Stringify call stack. */                              \
    ac_test_stack_to_str_(&ac_test_s_->scratch, ac_test_s_); \
    /* Print everything. */                                  \
    ac_test_print_str(ac_test_s_->scratch);                  \
    /* Statement evaluates to false. */                      \
    false;                                                   \
  })

// Fails the test case if 'cond' is false.
#define ac_test_expect_(cond, ...)                     \
  ({                                                   \
    ac_test_static_assert_begin_or_init_was_called_(); \
    ac_test_static_assert_params_complete_();          \
    const bool result = (cond);                        \
    if (!result) ac_test_fail(__VA_ARGS__);            \
    result;                                            \
  })

// Fails the test case if 'a op b' is false.
#define ac_test_op_(op, type, pri, a, b, ...)                                  \
  ({                                                                           \
    __typeof__(a) a_ = (a);                                                    \
    __typeof__(b) b_ = (b);                                                    \
    ac_test_expect(a_ op b_, "%s %s %s\n  %s: %" pri "\n  %s: %" pri, #a, #op, \
                   #b, #a, (type)a_, #b, (type)b_); /* Evalutes to T/F. */     \
  })

static inline void ac_test_begin_params_(ac_test_state* s) {
  ac_to_str(&s->names, "(");
  fprintf(s->out, "(");  // NOTE: Don't update print count.
}

static inline void ac_test_end_params_(ac_test_state* s) {
  ac_to_str(&s->names, " )");
  fprintf(s->out, " )");  // NOTE: Don't update print count.
}

static inline void ac_test_log_param_direct_str_(ac_test_state* s, ac_str str) {
  ac_to_str(&s->names, "%.*s", (int)str.len, str.data);
  ac_fprint(s->out, str);  // NOTE: Don't update print count.
}

#define ac_test_log_param_name_fmt_val_(name, fmt, value)           \
  ({                                                                \
    ac_test_static_assert_begin_with_params_was_called_();          \
    ac_test_static_assert_end_params_not_called_();                 \
    /* Stringify the parameter */                                   \
    ac_test_s_->scratch.len = 0;                                    \
    ac_to_str(&ac_test_s_->scratch, " %s: " fmt, name, value);      \
    /* Log it */                                                    \
    ac_test_log_param_direct_str_(ac_test_s_, ac_test_s_->scratch); \
  })

//------------------------------------------------------------------------------
// Public API.
//------------------------------------------------------------------------------

// Call before any of your test functions to initialize test state.
#define ac_test_init(ac_test_opts)                \
  enum { ac_test_begin_or_init_was_called_ = 1 }; \
  ac_test_state ac_test_s_data_ = {};             \
  ac_test_state* ac_test_s_ = &ac_test_s_data_;   \
  ac_test_state_init_(ac_test_s_, ac_test_opts);

// Retreive the test state, for querying failures, etc.
#define ac_test_state() ac_test_s_

// Call after all of your test functions have run.
// Prints final stats and cleans up test state.
#define ac_test_done()                                 \
  ({                                                   \
    ac_test_static_assert_begin_or_init_was_called_(); \
    ac_test_print_stats_(ac_test_s_);                  \
    const bool success = !ac_test_s_->failures;        \
    ac_test_state_free_(ac_test_s_);                   \
    success;                                           \
  })

// Call first in each test case to set up test state.
#define ac_test_begin(ac_test_state_ptr_)                                      \
  enum { ac_test_begin_or_init_was_called_ = 1 };                              \
  ac_test_state* ac_test_s_ = ac_test_state_ptr_;                              \
  if (!ac_test_expect_(ac_test_s_->stack.len && ac_test_s_->expect_test_begin, \
                       "Calling a test function must be done with "            \
                       "'ac_test_run(...)' for call stack "                    \
                       "tracing,\nbut this test appears to have been called "  \
                       "directly.\n\nINCOMPLETE call stack below.")) {         \
    return;                                                                    \
  }                                                                            \
  ac_test_s_->expect_test_begin = false;                                       \
  ac_test_begin_call_(ac_test_s_, ac_test_source_loc_here());                  \
  (void)ac_test_s_

// Call instead of ac_test_begin before any 'ac_test_param...(...)'.
#define ac_test_begin_with_params(ac_test_state_ptr_) \
  ac_test_begin(ac_test_state_ptr_);                  \
  enum { ac_test_begin_with_params_was_called_ = 1 }; \
  ac_test_begin_params_(ac_test_state_ptr_);

// Call after all 'ac_test_param...(...)'.
#define ac_test_end_params()                             \
  ac_test_static_assert_begin_with_params_was_called_(); \
  enum { ac_test_end_params_was_called_ = 1 };           \
  ac_test_end_params_(ac_test_s_);

// Logs an int paramter and its variable name.
#define ac_test_log_parami(param) \
  ac_test_log_param_name_fmt_val_(#param, "%" PRIi64, (int64_t)(param))

// Logs a uint paramter and its variable name.
#define ac_test_log_paramu(param) \
  ac_test_log_param_name_fmt_val_(#param, "%" PRIu64, (uint64_t)(param))

// Logs a double parameter and its variable name.
#define ac_test_log_paramf(param) \
  ac_test_log_param_name_fmt_val_(#param, "%f", (double)(param))

// Logs a null-terminated const char* parameter and its variable name.
#define ac_test_log_param_cstr(param) \
  ac_test_log_param_name_fmt_val_(#param, "%s", (const char*)(param))

// Log an ac_str parameter and its variable name.
#define ac_test_log_param_str(param)                                  \
  ac_test_log_param_name_fmt_val_(#param, "%.*s", (int)((param).len), \
                                  (param).data)

// Logs a user-formatted parameter value and its variable name.
// 'value_fmt' is a printf-style format string to print 'param's value.
#define ac_test_log_param_fmt(value_fmt, param) \
  ac_test_log_param_name_fmt_val_(#param, value_fmt, param)

// Logs a parameter and its name by stringifying with a user-provided function.
// The function must be: void (*to_str_func)(ac_str*, const typeof(param)*)
#define ac_test_log_param_to_str_func(to_str_func, param)           \
  ({                                                                \
    ac_test_s_->scratch.len = 0;                                    \
    to_str_func(&ac_test_s_.scratch, (param));                      \
    ac_test_log_param_name_fmt_val_(#param, "%.*s",                 \
                                    (int)(ac_test_s_->scratch.len), \
                                    ac_test_s_->scratch.data);      \
  })

// Normally, log_param* macros will stringify the name of the input variable.
// This one does not. It logs the provided string as-is.
#define ac_test_log_param_direct_str(str)                  \
  ({                                                       \
    ac_test_static_assert_begin_with_params_was_called_(); \
    ac_test_static_assert_end_params_not_called_();        \
    ac_test_log_param_direct_str_(ac_test_s_, str)         \
  })

// Prints to the test output; arguments in the style of printf.
// NOTE: Updates internal state to ensure that ok/failure status is printed
// correctly without becoming interleaved with user or expectation output. If
// you must use another method to write to the output, call this function at
// least once (e.g. with empty-string) before the end of the current test case.
#define ac_test_print(...)                    \
  ({                                          \
    ac_test_static_assert_params_complete_(); \
    ac_test_print_(ac_test_s_, __VA_ARGS__);  \
  })

// Prints to the test output; arguments in the style of printf.
// See comments on 'ac_test_print' for intended usage.
#define ac_test_print_str(str)                \
  ({                                          \
    ac_test_static_assert_params_complete_(); \
    ac_test_print_str_(ac_test_s_, (str));    \
  })

// Call to run a test case or sub-case. Calls may be nested.
// Manages the call-stack and prints into about cases/sub-cases.
// The first parameter is the test function. Subsequent parameters are passed to
// the test function after the automatically-passed test state parameter.
#define ac_test_run(func_name, ...)                                            \
  ({                                                                           \
    ac_test_static_assert_begin_or_init_was_called_();                         \
    const ac_test_source_loc loc_ = ac_test_source_loc_here();                 \
    ac_test_before_call_(ac_test_s_, loc_);                                    \
    (func_name)(ac_test_s_, ##__VA_ARGS__);                                    \
    ac_test_expect_(ac_test_s_->stack.len && !ac_test_s_->expect_test_begin,   \
                    "A test function must call 'ac_test_begin(...)' for call " \
                    "stack tracing, but the callee hasn't done so.");          \
    ac_test_after_call_(ac_test_s_); /* Evaluates to 'true' or 'false' */      \
  })

// Fails the test case but doesn't return. Always evalutes to 'false'.
#define ac_test_fail(...) ac_test_fail_(__VA_ARGS__)

// Fails the test case if 'cond' is false.
#define ac_test_expect(cond, ...) ac_test_expect_(cond, __VA_ARGS__)

// The following macros evaluate to 'true' on success or 'false' on failure. On
// failure, they also print the arguments, the stack trace, a user-provided
// format-string args, and fail the current test case as well as its callers.
// The test function continues executing after a failure, allowing the
// aggregation of multiple failures.
//
// For instance, eqi i.e. "equality of signed integers" produces:
//   ac_test_eqi(3, 3) => true
//   ac_test_eqi(3, 4) => false, fails test case, prints args and stack.
//
// Each "..." argument accepts an optional format string and printable args in
// the style of printf.
//
// To stop the execution of the test function after a failure, simply return:
//   if (!ac_test_eqi(3, 4)) return;
//

// Signed integer comparisons.
#define ac_test_eqi(a, b, ...) \
  ac_test_op_(==, int64_t, PRIi64, a, b, __VA_ARGS__)
#define ac_test_nei(a, b, ...) \
  ac_test_op_(!=, int64_t, PRIi64, a, b, __VA_ARGS__)
#define ac_test_lti(a, b, ...) \
  ac_test_op_(<, int64_t, PRIi64, a, b, __VA_ARGS__)
#define ac_test_gti(a, b, ...) \
  ac_test_op_(>, int64_t, PRIi64, a, b, __VA_ARGS__)
#define ac_test_lei(a, b, ...) \
  ac_test_op_(<=, int64_t, PRIi64, a, b, __VA_ARGS__)
#define ac_test_gei(a, b, ...) \
  ac_test_op_(>=, int64_t, PRIi64, a, b, __VA_ARGS__)

// Unsigned integer comparisons.
#define ac_test_equ(a, b, ...) \
  ac_test_op_(==, uint64_t, PRIu64, a, b, __VA_ARGS__)
#define ac_test_neu(a, b, ...) \
  ac_test_op_(!=, uint64_t, PRIu64, a, b, __VA_ARGS__)
#define ac_test_ltu(a, b, ...) \
  ac_test_op_(<, uint64_t, PRIu64, a, b, __VA_ARGS__)
#define ac_test_gtu(a, b, ...) \
  ac_test_op_(>, uint64_t, PRIu64, a, b, __VA_ARGS__)
#define ac_test_leu(a, b, ...) \
  ac_test_op_(<=, uint64_t, PRIu64, a, b, __VA_ARGS__)
#define ac_test_geu(a, b, ...) \
  ac_test_op_(>=, uint64_t, PRIu64, a, b, __VA_ARGS__)

// Unsigned integer comparisons. Evaluates to the the boolean result of the op.
#define ac_test_eqf(a, b, ...) ac_test_op_(==, double, "f", a, b, __VA_ARGS__)
#define ac_test_nef(a, b, ...) ac_test_op_(!=, double, "f", a, b, __VA_ARGS__)
#define ac_test_ltf(a, b, ...) ac_test_op_(<, double, "f", a, b, __VA_ARGS__)
#define ac_test_gtf(a, b, ...) ac_test_op_(>, double, "f", a, b, __VA_ARGS__)
#define ac_test_lef(a, b, ...) ac_test_op_(<=, double, "f", a, b, __VA_ARGS__)
#define ac_test_gef(a, b, ...) ac_test_op_(>=, double, "f", a, b, __VA_ARGS__)

#endif  // AC_TEST_H_
