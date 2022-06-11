#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"

int
main(void)
  {
    getprocs();

    exit(1);
  }
