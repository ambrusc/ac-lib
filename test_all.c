#include "test_all.h"

#include "ac_alloc.h"
#include "ac_test.h"
#include "ac_test_test.h"

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  ac_test_init((ac_test_opts){});
  ac_test_run(ac_test_test);
  ac_test_fail("Seeing if CI picks up this failure.");
  return ac_test_done() ? 0 : 1;
}
