#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int len(char *str);
int str2int(char *str, char *table);

int main(int argc, char *argv[]){
  if(argc < 2){
    fprintf(2, "Usage: sleep times...\n");
    exit(1);
  }
  
  char table[] = "0123456789";
    
  sleep(str2int(argv[1], table));
  exit(0);
  return 0;
}

int len(char * str){
  int i = 0;
  for (;;i++){
    if (str[i] == '\x00'){
      break;
    }
  }
  return i;
}

int str2int(char *str, char *table){
  int num = 0;
  int flags = 0;
  for (int i = 0;i < len(str);i++){
    for (int j = 0;j < len(table);j++){
      if (str[i] == table[j]){
        flags = 1;
        num += j;
        num *= len(table);
      }
    }
  }
  if (flags == 0){
    return -1;
  }

  return num;
}
