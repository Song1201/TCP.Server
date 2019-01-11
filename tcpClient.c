#include <netdb.h>
#include "common.h"
#include <stdio.h>

#define DEST_PORT 2000
#define SERVER_IP_ADDRESS "127.0.0.1" 

testStructType clientData;
resultStructType result;

void setupTcpCommunication() {
  int sockFd = 0, sentRecvBytes = 0;
  int addrLen = sizeof(struct sockaddr);
  struct sockaddr_in dest;
  dest.sin_family = AF_INET;
  dest.sin_port = DEST_PORT;
  struct hostent *host = (struct hostent*)gethostbyname(SERVER_IP_ADDRESS);
  dest.sin_addr = *((struct in_addr*)host->h_addr_list[0]);
  sockFd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  connect(sockFd,(struct sockaddr*)&dest,addrLen);
  while (1) {
    printf("Enter a: ");
    scanf("%u",&clientData.a);
    printf("Enter b: ");
    scanf("%u",&clientData.b);

    sentRecvBytes = sendto(sockFd,&clientData,sizeof(testStructType),0,
      (struct sockaddr*)&dest,addrLen);
    printf("Number of bytes sent = %d\n",sentRecvBytes);
    sentRecvBytes = recvfrom(sockFd,(char*)&result,sizeof(resultStructType),0,
      (struct sockaddr*)&dest,(socklen_t*)&addrLen);
    if (sentRecvBytes == 0) {
      printf("Connection to server closed.\n");
      return;
    }
    printf("Number of bytes received = %d\n",sentRecvBytes);
    printf("Result received = %u\n",result.c);
  }
}

int main(int argc, char const *argv[]) {
  setupTcpCommunication();
  printf("Application quits.\n");
  return 0;
}
