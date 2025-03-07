#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
  int pipefd[2];
  if (pipe(pipefd) == -1){
    fprintf(2, "create pipe failed");
    exit(0);
  }
  int prime[100] = {2, 3, 5, 7, 11, 13, 15, 17, 19, 23, 29, 31, 37, 41, 47};
  int idx = 0;
  while(1){
    int fpid = fork();
    sleep(1);
    if (fpid < 0){
      fprintf(2, "fork failed");
      exit(0);
    }

    if (fpid == 0){
      read(pipefd[0], &idx, 1);
      close(pipefd[0]);
      
      printf("prime %d\n", prime[idx]);

      exit(0);
    }


    else{
      
      write(pipefd[1], &idx, 1);
      if (idx == 11) exit(0); 
      idx ++;
      close(pipefd[1]);
    }
  }
}



