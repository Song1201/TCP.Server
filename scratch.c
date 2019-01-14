#include <arpa/inet.h> // To use inet_ntoa
#include <memory.h> // To use memset
#include <sys/select.h> // To use fd_set
#include <stdio.h> // To use printf
#include <errno.h> // To use errno
#include "common.h" // Message stucture
#include <stdlib.h> // To use exit
#include <unistd.h> // To use close
#include <search.h> // To use hcreate(), hsearch()
#include <string.h> // To use strcpy()

#define ADDR_LEN sizeof(struct sockaddr)

int main(int argc, char const *argv[]) {
  // char a[5];
  // memset(a,0,1);
  // memset(a+1,'a',1);
  // memset(a+2,'b',1);
  // a[2] = 0;
  // for (int i=0; i<5; i++) {
  //   printf("%hu  %c\n",a[i],a[i]);
  // }
  printf("%lu\n",ADDR_LEN);

}
