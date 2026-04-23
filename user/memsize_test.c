#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int before, after_alloc, after_free;
  int bytes = 20 * 1024;  // 20 KB

  before = memsize();
  printf("memsize before: %d bytes\n", before);

  char *buf = (char *) malloc(bytes);
  if(buf == 0){
    printf("malloc failed\n");
    exit(1);
  }

  after_alloc = memsize();
  printf("memsize after alloc: %d bytes\n", after_alloc);

  free(buf);

  after_free = memsize();
  printf("memsize after free: %d bytes\n", after_free);

  exit(0);
}