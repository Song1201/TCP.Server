#include <arpa/inet.h> // To use inet_ntoa
#include <memory.h> // To use memset
#include <sys/select.h> // To use fd_set
#include <stdio.h> // To use printf
#include <errno.h> // To use errno
#include "common.h" // Message stucture
#include <stdlib.h> // To use exit
#include <unistd.h> // To use close
#include <search.h> // To use hcreate(), hsearch()

#define MAX_CLIENT_SUPPORTED 32
#define MAX_USERNAME_LEN 20
#define PASSWORD_LEN 256
#define MAX_NUM_USER 10000

char dataBuffer[1024];
int monitoredFdSet[MAX_CLIENT_SUPPORTED];

static void initializeMonitorFdSet() {
  for (int i = 0;i < MAX_CLIENT_SUPPORTED;i++) 
    monitoredFdSet[i] = -1;
}

static void addToMonitoredFdSet(int sockFd) {
  for (int i = 0;i < MAX_CLIENT_SUPPORTED;i++) {
    if (monitoredFdSet[i] != -1) continue;
    monitoredFdSet[i] = sockFd;
    return;
  }
}

static void reInitReadFds(fd_set *fdSetPtr) {
  FD_ZERO(fdSetPtr);
  for (int i = 0;i < MAX_CLIENT_SUPPORTED;i++) {
    if (monitoredFdSet[i] != -1) FD_SET(monitoredFdSet[i],fdSetPtr);
  }
}

static int getMaxFd() {
  int max = -1;
  for (int i = 0;i < MAX_CLIENT_SUPPORTED;i++) {
    if (monitoredFdSet[i] > max) max = monitoredFdSet[i];
  }
  return max;
}

static void rmFromMonitoredFdSet(int sockFd) {
  for (int i=0;i<MAX_CLIENT_SUPPORTED;i++) {
    if (monitoredFdSet[i] == sockFd) {
      monitoredFdSet[i] = -1;
      return;
    }
  }
}

void setupTcpServerCommunication(unsigned short SERVER_PORT) {
  int masterSockTcpFd = 0, sentRecvBytes = 0, addrLen = 0;
  int commSockFd = 0;
  fd_set readFds;

  struct sockaddr_in serverAddr,clientAddr;
  initializeMonitorFdSet();

  if ((masterSockTcpFd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1) {
    printf("Master socket creation failed!\n");
    exit(1);
  }

  serverAddr.sin_family = AF_INET; 
  serverAddr.sin_port = SERVER_PORT;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  addrLen = sizeof(struct sockaddr);

  if (bind(masterSockTcpFd,(struct sockaddr*)&serverAddr,addrLen) == -1) {
    printf("Master socket bind failed!\n");
    return;
  }

  if (listen(masterSockTcpFd,5) < 0) {
    printf("listen failed!\n");
    return; 
  }
  addToMonitoredFdSet(masterSockTcpFd);

  while (1) {
    reInitReadFds(&readFds);
    printf("Blocked on select system call...\n");

    select(getMaxFd()+1,&readFds,NULL,NULL,NULL);
    if (FD_ISSET(masterSockTcpFd,&readFds)) {   
      printf("New connection received. Accept the connection. Client and " 
        "server completes TCP 3-way handshake at this point.\n");
      commSockFd = accept(masterSockTcpFd,(struct sockaddr*)&clientAddr,
        (socklen_t*)&addrLen);
      if (commSockFd < 0) {
        printf("Accept error: errno = %d\n",errno);
        exit(0);
      }
      addToMonitoredFdSet(commSockFd);
      printf("Connection acceptted from client: %s:%u\n",
        inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
    } else {
      int commSockFd = -1;
      for (int i=0;i<MAX_CLIENT_SUPPORTED;i++) {
        if (FD_ISSET(monitoredFdSet[i],&readFds)) {
          commSockFd = monitoredFdSet[i];
          memset(dataBuffer,0,sizeof(dataBuffer));
          sentRecvBytes = recvfrom(commSockFd,dataBuffer,sizeof(dataBuffer),0,
            (struct sockaddr*)&clientAddr, (socklen_t*)&addrLen);
          printf("Server received %d bytes from client %s:%u\n",sentRecvBytes,
            inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
          if (sentRecvBytes == 0) {
            close(commSockFd);
            rmFromMonitoredFdSet(commSockFd);
            break;
          }
          testStructType *clientData = (testStructType*)dataBuffer;
          if (clientData->a == 0 && clientData->b == 0) {
            close(commSockFd);
            rmFromMonitoredFdSet(commSockFd);
            printf("Server closes connection with client: %s:%u\n",inet_ntoa(
              clientAddr.sin_addr),ntohs(clientAddr.sin_port));
            break;
          }
          resultStructType result;
          result.c = clientData->a + clientData->b;
          sentRecvBytes = sendto(commSockFd,(char*)&result,
            sizeof(resultStructType),0,(struct sockaddr*)&clientAddr,
            sizeof(struct sockaddr));
          printf("Server sent %d bytes in reply to client.\n",sentRecvBytes);

        }
      }
    }
  }
}

int main(int argc, char const *argv[])
{
  FILE *confPtr = fopen("server.conf","r");

  // Read server port
  unsigned short SERVER_PORT;
  int fileState = fscanf(confPtr,"%hu",&SERVER_PORT);
  printf("%hu\n",SERVER_PORT);

  // Put all usernames and passwords into a hash table
  int userInfo = hcreate(MAX_NUM_USER);
  char username[MAX_USERNAME_LEN+1];
  char password[PASSWORD_LEN];
  while (fscanf(confPtr,"%s",username) != EOF) {
    fscanf(confPtr,"%s",password);
    ENTRY userEntry = {.key=username, .data=password};
    hsearch(userEntry,ENTER);
  }
  fclose(confPtr);

  // setupTcpServerCommunication(SERVER_PORT);
  return 0;
}
