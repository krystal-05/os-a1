#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

static struct uproc procs[NPROC];

static char *
state_name(int state)
{
  static char *names[] = {
    "unused", "used", "sleeping", "runnable", "running", "zombie"
  };
  if (state < 0 || state >= (int)(sizeof(names) / sizeof(names[0])))
    return "unknown";
  return names[state];
}

int
main(void)
{
  int n = getprocs(procs, NPROC);
  if (n < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }

  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");
  for (int i = 0; i < n; i++)
    printf("%d\t%d\t%s\t%lu\t%s\n", procs[i].pid, procs[i].ppid,
           state_name(procs[i].state), procs[i].sz, procs[i].name);
  exit(0);
}
