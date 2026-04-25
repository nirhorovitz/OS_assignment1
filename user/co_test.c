#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define ITERS 5

// NOTE on termination: every successful co_yield leaves the SENDER
// parked SLEEPING on the co_yield channel. So after a bounded number
// of exchanges, exactly one of the two procs is still parked and can
// only be woken by the *other* proc calling co_yield again. There is
// no clean way to break out of this from userspace alone, so this
// basic test does ITERS exchanges and then INTENTIONALLY hangs.
// Watch the prints, then exit qemu with: Ctrl-A x
static void
basic_test(void)
{
  int pid1 = getpid();
  int pid2 = fork();

  if(pid2 < 0){
    printf("fork failed\n");
    exit(1);
  }

  if(pid2 == 0){
    // Child: yields value 1 to parent, expects 2 back.
    for(int i = 0; i < ITERS; i++){
      int value = co_yield(pid1, 1);
      printf("Child  received: %d (expected 2)\n", value);
      if(value != 2){
        printf("BASIC TEST FAILED in child: got %d\n", value);
        exit(1);
      }
    }
    exit(0);
  } else {
    // Parent: yields value 2 to child, expects 1 back.
    for(int i = 0; i < ITERS; i++){
      int value = co_yield(pid2, 2);
      printf("Parent received: %d (expected 1)\n", value);
      if(value != 1){
        printf("BASIC TEST FAILED in parent: got %d\n", value);
        exit(1);
      }
    }
    exit(0);
  }
}

static void
error_tests(void)
{
  int r;

  // (a) yield to a non-existent PID.
  r = co_yield(9999, 1);
  printf("co_yield(non-existent pid)  -> %d (expected -1)\n", r);
  if(r != -1) printf("ERROR TEST (a) FAILED\n");

  // (b) yield to self.
  r = co_yield(getpid(), 1);
  printf("co_yield(self)              -> %d (expected -1)\n", r);
  if(r != -1) printf("ERROR TEST (c) FAILED\n");

  // (c) invalid pid (zero / negative).
  r = co_yield(0, 1);
  printf("co_yield(0, 1)              -> %d (expected -1)\n", r);
  if(r != -1) printf("ERROR TEST (d) FAILED\n");

  r = co_yield(-3, 1);
  printf("co_yield(-3, 1)             -> %d (expected -1)\n", r);
  if(r != -1) printf("ERROR TEST (e) FAILED\n");

  // (d) yield to a killed process.
  int pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }
  if(pid == 0){
    // Child just exits immediately so the pid becomes invalid/killed.
    exit(0);
  }
  wait(0);
  r = co_yield(pid, 1);
  printf("co_yield(killed pid)        -> %d (expected -1)\n", r);
  if(r != -1) printf("ERROR TEST (b) FAILED\n");

  printf("ERROR TESTS DONE\n");
}

int
main(int argc, char *argv[])
{
  // Error tests first, because basic_test() hangs by design at the end.
  printf("=== co_yield error tests ===\n");
  error_tests();

  printf("=== co_yield basic test ===\n");
  basic_test();

  exit(0);
}
