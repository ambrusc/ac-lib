#include "ac_time_test.h"

#include "ac_test.h"

#include <stdio.h>

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  ac_test_init((ac_test_opts){});
  ac_test_run(ac_time_test);
  return ac_test_done() ? 0 : 1;
}