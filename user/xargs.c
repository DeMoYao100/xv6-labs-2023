#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char * readline(){
  char *buf;
  char *buf_base;
  buf = (char*) malloc(100);
  buf_base = buf;
  while(1){
    read(0, buf, 1);
    if (*buf == '\n' || *buf == '\x00'){
      break;
    }
    buf++;
  }
  *buf = '\x00';
  return buf_base;
}



int main(int argc,char *argv[]){
  int flags = 1;
//  int lines = 1;
  if (*argv[1] == '-' && *argv[1]+1 == 'n'){
//    lines = atoi(argv[2][0]);
    flags = 3;
  }

  char *new_args[0x10];
  int i=0;
  for (;i<argc;i++){
    new_args[i] = argv[i+flags];
  }
  i --;
  new_args[i] = "\x00"; 
  while(1){
    char *tmp_buf = readline();
    if (*tmp_buf == '\x00') break;
    new_args[i++] = tmp_buf; 
  }
  new_args[i] = '\x00';
  for (int j=0;j<i;j++){
//    printf("args:%s\n", new_args[j]);
  }

  int pid = fork();
  if (pid == 0){
    exec(argv[1], new_args);
    exit(0);
  }
  else if (pid > 0){
    wait(0);
  }
  else {
    fprintf(2, "fork failed\n");
    exit(1);
  }

}
