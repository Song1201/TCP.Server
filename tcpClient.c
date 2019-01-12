#include <netdb.h> // To use sockaddr
#include "common.h" // Message structure
#include <stdio.h> // To use printf()
#include <arpa/inet.h> // To use inet_pton()
#include <string.h> // To use strlen()
#include <stdlib.h> // To use exit()
#include <fcntl.h> // To use fcntl()
#include <sys/select.h> // To use fd_set
#include <time.h> // To use timeval
#include <errno.h> // To use errno

#define IPV4_ADDR_MAX_LEN 15
#define TIMEOUT 20
#define ADDR_LEN sizeof(struct sockaddr)

testStructType clientData;
resultStructType result;

struct sockaddr_in setupDestIPv4() {
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
  printf("Please input server port number: ");
  unsigned short DEST_PORT;
  scanf("%hu",&DEST_PORT);
  dest.sin_port = DEST_PORT;
  return dest;
}

int connectToDest(const struct sockaddr_in dest) {
  int sockFd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  // int addrLen = sizeof(struct sockaddr);
  printf("Conneting to server... It takes at most 3 minutes.\n\n");
  if (connect(sockFd,(struct sockaddr*)&dest,ADDR_LEN) != 0) {
    printf("Connection to server failed.\nFailed server IPv4 address: %s\n"
      "Failed server port number: %hu\n",inet_ntoa(dest.sin_addr),
      dest.sin_port);
    printf("Exiting program.\n\n");
    exit(1);
  }
  printf("Connected to server.\nConnected server IPv4 address: %s\n"
    "Connected server port number: %hu\n\n",inet_ntoa(dest.sin_addr),
    dest.sin_port);
  return sockFd;
}

void setupTcpCommunication() {
  struct sockaddr_in dest = setupDestIPv4();
  int sockFd = connectToDest(dest);
  
  while (1) {
    printf("Enter a: ");
    scanf("%u",&clientData.a);
    printf("Enter b: ");
    scanf("%u",&clientData.b);
    int sentRecvBytes = sendto(sockFd,&clientData,sizeof(testStructType),0,
      (struct sockaddr*)&dest,ADDR_LEN);
    printf("Number of bytes sent = %d\n",sentRecvBytes);
    int addrLen = ADDR_LEN;
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
