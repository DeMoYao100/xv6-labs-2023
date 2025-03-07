#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

int match(char *path, char *name){
  for (int i=0;i<strlen(path);i++){
    int j=0;
    for (;j<strlen(name);j++){
      if (path[i+j] != name[j]){
        break;
      }
    }
    if (j == strlen(name)){
      return 1;
    }
  }
  return 0;
}

void find(char *path, char *name){
  char buf[0x100], *p;
  int fd;
  struct dirent de;
  struct stat st;
  if ((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return ;
  }

  if (fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return ;
  }

  switch(st.type){
    case T_DEVICE:
    case T_FILE:
      if (match(path, name)){
        printf("%s\n", path);
      }
        break;
    case T_DIR:

      
      if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        printf("find: path too long\n");
        break;
      }

      strcpy(buf, path);
      p = buf+strlen(buf);
      *p++ = '/';
      while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if (de.inum == 0) continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (match(path, name))
          printf("%s\n", buf);
        if (buf[strlen(buf) - 1] == '.') continue;
        find(buf, name);
      }
  }
  
  close(fd);
  
}


int main(int argc, char *argv[]){
  if (argc < 2){
      exit(0);
  }
  find(argv[1], argv[2]);
}


