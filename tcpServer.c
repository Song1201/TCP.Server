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

#define MAX_CLIENT_SUPPORTED 32
#define PASSWORD_HASH_LEN 256
#define MAX_NUM_USER 10000
#define EMPTY 0
#define CONNECTED 1
#define VERIFIED 2
#define ADDR_LEN sizeof(struct sockaddr)
#define MAX_CONNECT_REQUEST 5
#define NON_EXIST -1


static void addToFdArray(int sockFd, int *fdArray) {
  for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
    if (fdArray[i] == NON_EXIST) {
      fdArray[i] = sockFd;
      return;
    }
  }
}

static int getMaxFd(const int masterSockFd, const int *fdArray) {
  int max = masterSockFd;
  for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
    if (fdArray[i]>max) max = fdArray[i];
  }
  return max;
}

static void rmFromFdArray(int sockFd, int *fdArray) {
  for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
    if (fdArray[i] == sockFd) {
      fdArray[i] = -1;
      return;
    }
  }
}

static char *strdup(const char *origin) {
  char *copy = malloc(sizeof(origin));
  strcpy(copy, origin);
  return copy;
}

static unsigned short loadServerConf() {
  FILE *confPtr = fopen("server.conf","r");

  unsigned short SERVER_PORT;
  fscanf(confPtr,"%hu",&SERVER_PORT);

  if (hcreate(MAX_NUM_USER) == 0) {
    printf("Failed to create hash table for user info.\n");
    exit(1);
  } 
    
  char username[MAX_USERNAME_LEN+1];
  char passwordHash[PASSWORD_HASH_LEN+1];
  while (fscanf(confPtr,"%s",username) != EOF) {
    fscanf(confPtr,"%s",passwordHash);
    ENTRY userEntry = {.key=strdup(username), .data=strdup(passwordHash)};
    hsearch(userEntry,ENTER);
  }
  fclose(confPtr);

  return SERVER_PORT;
}



static int createMasterSocket(const unsigned short SERVER_PORT) {
  int masterSockFd;
  if ((masterSockFd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1) {
    printf("Failed to create master socket!\n");
    exit(1);
  }

  // AF_INET for using IPv4 address, INADDR_ANY for later binding the socket to
  // all available local interfaces. 
  struct sockaddr_in serverAddr = {.sin_family=AF_INET, .sin_port=SERVER_PORT, 
    .sin_addr.s_addr=INADDR_ANY};

  if (bind(masterSockFd,(struct sockaddr*)&serverAddr,ADDR_LEN) == -1) {
    printf("Failed to bind master socket!\n");
    exit(1);
  }

  if (listen(masterSockFd, MAX_CONNECT_REQUEST) < 0) {
    printf("Failed to listen!\n");
    exit(1);
  }  

  return masterSockFd;
}

static void initClientSockets(int *fdArray, char *fdStatus) {
  for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
    fdArray[i] = NON_EXIST;
    fdStatus[i] = EMPTY;
  }
}

static void initFdSet(fd_set *fdSetPtr, int masterSockFd, int *fdArray) {
  FD_ZERO(fdSetPtr);
  FD_SET(masterSockFd, fdSetPtr);
  for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
    if (fdArray[i] != NON_EXIST) FD_SET(fdArray[i], fdSetPtr);
  } 
} 

static void acceptConnection(int masterSockFd, int *fdArray) {
  printf("New connection received.\n");
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = ADDR_LEN;
  int commSockFd = accept(masterSockFd, (struct sockaddr*)&clientAddr,
    &clientAddrLen);
  printf("After acception: %d\n",clientAddrLen);
  if (commSockFd<0) {
    printf("Accept error: errno = %d\n",errno);
    exit(1);
  }
  addToFdArray(commSockFd,fdArray);
  printf("Connection accepted from client: %s:%u\n",
    inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
}

static void receiveData(const int commSockFd, struct sockaddr_in *clientAddrPtr, 
char *data) {
  socklen_t clientAddrLen = ADDR_LEN;
  getpeername(commSockFd, (struct sockaddr*)clientAddrPtr,&clientAddrLen); 
  int sentRecvBytes = recvfrom(commSockFd, data, sizeof(data), 0, NULL, NULL);
  printf("Server received %d bytes from client %s:%u\n",sentRecvBytes,
    inet_ntoa(clientAddrPtr->sin_addr),ntohs(clientAddrPtr->sin_port));
}

int main(int argc, char const *argv[]) {
  // make login tlv
  // char *login = "lsc\n9999\n";
  // tlv loginInfo = {.type=LOGIN, .length=MAX_USERNAME_LEN+MAX_PASSWORD_LEN, 
  //   .value=login};

  // printf("%d\n%d\n%s\n",loginInfo.type,loginInfo.length,loginInfo.value);

  const unsigned short SERVER_PORT = loadServerConf();
  int masterSockFd = createMasterSocket(SERVER_PORT);
  int fdArray[MAX_CLIENT_SUPPORTED];
  char fdStatus[MAX_CLIENT_SUPPORTED];
  initClientSockets(fdArray, fdStatus);

  while (1) {
    fd_set fdSet;
    initFdSet(&fdSet, masterSockFd, fdArray);
    printf("Blocked on select system call...\n");
    select(getMaxFd(masterSockFd, fdArray)+1, &fdSet, NULL, NULL, NULL);

    if (FD_ISSET(masterSockFd, &fdSet))
      acceptConnection(masterSockFd, fdArray);
    else {
      for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
        if (FD_ISSET(fdArray[i], &fdSet)) {
          int commSockFd = fdArray[i];
          struct sockaddr_in clientAddr;
          char data[MAX_DATA_SIZE]; // Later change it to MAX_TLV_SIZE
          receiveData(commSockFd, &clientAddr, data);

          testStructType *clientData = (testStructType*)data;
          if (clientData->a == 0 && clientData->b == 0) {
            close(commSockFd);
            rmFromFdArray(commSockFd, fdArray);
            printf("Server closes connection with client: %s:%u\n",inet_ntoa(
              clientAddr.sin_addr),ntohs(clientAddr.sin_port));
            break;
          }
          resultStructType result;
          result.c = clientData->a + clientData->b;
         
          int sentRecvBytes = sendto(commSockFd,(char*)&result,
            sizeof(resultStructType),0,(struct sockaddr*)&clientAddr,
            ADDR_LEN);
          printf("Server sent %d bytes in reply to client.\n",sentRecvBytes);                    
        }
      }
    }
  }

  return 0;
}

// DO NOT DELETE THESE COMMENTS!!!!!!!!!!!!!!!!!!!!!
// DO NOT DELETE THESE COMMENTS!!!!!!!!!!!!!!!!!!!!!
// DO NOT DELETE THESE COMMENTS!!!!!!!!!!!!!!!!!!!!!
// Uncomment this function to test the hash table used for storing usernames and 
//  password hash values. Call this function after calling loadServerConf().
// static void testHashTable() {
//   printf("Testing Hash table...\nPlease enter an username: ");
//   char username[MAX_USERNAME_LEN+1];
//   fgets(username,sizeof(username),stdin);
//   username[strlen(username)-1] = '\0'; // Remove the trailing '\n' from stdin
//   ENTRY query = {.key=username};
//   ENTRY *entry = hsearch(query,FIND);
//   if (entry == NULL) printf("Username doesn't exist.\n");
//   else printf("Username: %s  Password: %s\n",entry->key,(char*)entry->data);
//   printf("Test done.\n");
// }