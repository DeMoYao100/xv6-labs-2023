#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
  int pipefd1[2], pipefd2[2];
  char buf[0x100];
  if (pipe(pipefd1) == -1 && pipe(pipefd2) == -1){
    fprintf(2, "create pipe failed");
    exit(0);
  }


  int fpid = fork();
  if (fpid < 0){
    fprintf(2, "fork failed");
    exit(0);
  }
  if (fpid == 0){
    read(pipefd1[0], buf, 4);
    printf("%d: received %s\n", getpid(), buf);
    write(pipefd1[1], "pong", 4);
  }

  else{
    write(pipefd1[1], "ping", 4);
    read(pipefd1[0], buf, 4);
    printf("%d: received %s\n", getpid(), buf);
  }


}
