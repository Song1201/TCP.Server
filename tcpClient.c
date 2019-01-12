#include <netdb.h> // To use sockaddr
#include "common.h" // Message structure
#include <stdio.h> // To use printf()
#include <arpa/inet.h> // To use inet_pton()
#include <string.h> // To use strlen()
#include <stdlib.h> // To use exit()

#define IPV4_ADDR_MAX_LEN 15

testStructType clientData;
resultStructType result;

struct sockaddr_in setupDestIp() {
  // sockaddr_in means destination(server) address is IPv4 address
  struct sockaddr_in dest; 
  dest.sin_family = AF_INET; // Address Family is IPv4 address
  printf("Please input server IPv4 address: ");
  char SERVER_IPV4_ADDR[IPV4_ADDR_MAX_LEN+1];
  fgets(SERVER_IPV4_ADDR,sizeof(SERVER_IPV4_ADDR),stdin);
  // Remove the '\n' from input string.
  SERVER_IPV4_ADDR[strlen(SERVER_IPV4_ADDR)-1] = '\0';  
  if (inet_pton(AF_INET,SERVER_IPV4_ADDR, &(dest.sin_addr)) != 1) {
    printf("Failed to read server IPv4 address. Exiting program.\n");
    exit(1);
  }
  printf("Please input server port: ");
  unsigned short DEST_PORT;
  scanf("%hd",&DEST_PORT);
  dest.sin_port = DEST_PORT;
  return dest;
}

void setupTcpCommunication() {
  struct sockaddr_in dest = setupDestIp();

  int sockFd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  int addrLen = sizeof(struct sockaddr);
  connect(sockFd,(struct sockaddr*)&dest,addrLen);
  while (1) {
    printf("Enter a: ");
    scanf("%u",&clientData.a);
    printf("Enter b: ");
    scanf("%u",&clientData.b);

    int sentRecvBytes = sendto(sockFd,&clientData,sizeof(testStructType),0,
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
