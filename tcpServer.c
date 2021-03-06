#include <arpa/inet.h> // To use inet_ntoa
#include <memory.h> // To use memset
#include <sys/select.h> // To use fd_set
#include <stdio.h> // To use printf
#include <errno.h> // To use errno
#include "common.h" // Shared part of tcpServer.c and tcpClient.c
#include <stdlib.h> // To use exit
#include <unistd.h> // To use close
#include <search.h> // To use hcreate(), hsearch()
#include <string.h> // To use strcpy()
#include "hashPassword.h"

#define MAX_CLIENT_SUPPORTED 32
#define MAX_NUM_USER 10000
#define MAX_CONNECT_REQUEST 5
#define NON_EXIST -1
#define CONNECTED 1
#define VERIFIED 2

static void addToFdArray(int sockFd, int *fdArray, char *fdStatus) {
  for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
    if (fdArray[i] == NON_EXIST) {
      fdArray[i] = sockFd;
      fdStatus[i] = CONNECTED;
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

static unsigned char *memDup(const unsigned char *origin, const unsigned int length) {
  unsigned char *copy = malloc(length);
  memcpy(copy, origin, length);
  return copy;
}

static void readUserInfo(FILE *confPtr) {
  if (hcreate(MAX_NUM_USER) == 0) {
    printf("Failed to create hash table for user info.\n");
    exit(1);
  } 
  char username[MAX_USERNAME_LEN+1];
  while (fscanf(confPtr,"%s",username) != EOF) {
    unsigned char pwDigest[PW_DIGEST_LEN];
    char pwDigestString[PW_DIGEST_LEN*2];
    fscanf(confPtr,"%s",pwDigestString);
    // Convert password digest hex format string to real binary digest
    for (int i=0; i<PW_DIGEST_LEN; i++) {
      sscanf(pwDigestString+2*i, "%2x", (unsigned int*)(pwDigest+i));
    }
    ENTRY userEntry = {.key=(char*)memDup((unsigned char*)username, 
      strlen(username)), .data=memDup(pwDigest, PW_DIGEST_LEN)};
    hsearch(userEntry,ENTER);
  }
  fclose(confPtr);
}

static unsigned short loadServerConf() {
  FILE *confPtr = fopen("server.conf","r");
  unsigned short SERVER_PORT;
  fscanf(confPtr,"%hu",&SERVER_PORT);
  readUserInfo(confPtr);
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
    fdStatus[i] = NON_EXIST;
  }
}

static void initFdSet(fd_set *fdSetPtr, int masterSockFd, int *fdArray) {
  FD_ZERO(fdSetPtr);
  FD_SET(masterSockFd, fdSetPtr);
  for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
    if (fdArray[i] != NON_EXIST) FD_SET(fdArray[i], fdSetPtr);
  } 
} 

static void acceptConnection(int masterSockFd, int *fdArray, char *fdStatus) {
  printf("New connection received.\n");
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = ADDR_LEN;
  int clientFd = accept(masterSockFd, (struct sockaddr*)&clientAddr,
    &clientAddrLen);
  if (clientFd<0) {
    printf("Accept error: errno = %d\n",errno);
    exit(1);
  }
  addToFdArray(clientFd, fdArray, fdStatus);
  printf("Connection accepted from client: %s:%u\n",
    inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
}

static int receiveData(const int clientFd, struct sockaddr_in *clientAddrPtr, 
char *data) {
  socklen_t clientAddrLen = ADDR_LEN;
  getpeername(clientFd, (struct sockaddr*)clientAddrPtr,&clientAddrLen); 
  int sentRecvBytes = recvfrom(clientFd, data, MAX_DATA_SIZE, 0, NULL, NULL);
  printf("Server received %d bytes from client %s:%u\n",sentRecvBytes,
    inet_ntoa(clientAddrPtr->sin_addr),ntohs(clientAddrPtr->sin_port));
  return sentRecvBytes;
}

static void closeConnection(int *clientFdPtr, char *clientStatusPtr, 
const struct sockaddr_in *clientAddrPtr) {
  close(*clientFdPtr);
  *clientFdPtr = *clientStatusPtr = NON_EXIST;
  printf("Server closed connection with client: %s:%u\n",inet_ntoa(
    clientAddrPtr->sin_addr),ntohs(clientAddrPtr->sin_port));
}

static void printNumConnectedClients(const int *fdArray) {
  unsigned int numConnectedClients = 0;
  for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
    if (fdArray[i] != NON_EXIST) numConnectedClients++;
  }
  printf("\nNumber of currently connected clients: %u\n", numConnectedClients);
}

static void verifyUser(char *username, const char *password, char *data, 
char *clientStatusPtr) {
  ENTRY query = {.key=username};
  ENTRY *entry = hsearch(query,FIND);
  if (entry == NULL) {
    printf("Username doesn't exist.\n");
    strcpy(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN, "Username doesn't exist!");  
    return;
  }
  unsigned char pwDigest[PW_DIGEST_LEN];
  hashPassword(password, pwDigest);
  if (memcmp(entry->data, pwDigest, PW_DIGEST_LEN) == 0) {
    *clientStatusPtr = VERIFIED;
    printf("User identity verified.\n");
    strcpy(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN, "Welcome to the TCP server " 
      "written by Song ^_^");
  } else {
    printf("Wrong password.\n");
    strcpy(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN, "Wrong password!");
  }
}

static void clientLogin(char *data, char *clientStatusPtr) {
  printf("Verifying client identity...\n");
  char username[MAX_USERNAME_LEN+1] = "";
  strcpy(username, data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN);
  char password[MAX_PASSWORD_LEN+1] = "";
  strcpy(password, data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN+MAX_USERNAME_LEN+1);
  printf("Received username: %s\nReceived password: ******\n", username);
  *data = TEXT;
  verifyUser(username, password, data, clientStatusPtr);
}

static void clientSum(char *data) {
  int a = *((int*)(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN));
  int b = *((int*)(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN+sizeof(a)));
  printf("a = %d  b = %d\n", a, b);
  int sum = a + b;
  memcpy(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN, &sum, sizeof(sum));
  printf("Result = %d\n", *((int*)(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN)));
}

static unsigned char processAndPrepareData(char *data, char *clientStatusPtr) {
  unsigned char type = (unsigned char) data[0];
  if (*clientStatusPtr == CONNECTED && type == LOGIN) {
    clientLogin(data, clientStatusPtr);
  } else if (*clientStatusPtr == VERIFIED && type == SUM) clientSum(data);
  else if (type == EXIT) {
    printf("Client sent exit signal. Reply confirm signal and "
      "close connection.\n");
  } else {
    printf("Client sent invalid data.\n");
    *data = TEXT;
    strcpy(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN, "Invalid data is "
      "received by server!");
  }
  return type;
}

static unsigned char processAndReply(char *data, char *clientStatusPtr, 
const int clientFd, struct sockaddr_in *clientAddrPtr) {
  unsigned char type = processAndPrepareData(data, clientStatusPtr);
  int sentRecvBytes = sendto(clientFd, data, MAX_DATA_SIZE, 0, 
    (struct sockaddr*)clientAddrPtr, ADDR_LEN);
  printf("Server replied %d bytes to client.\n", sentRecvBytes); 
  return type;
}

static void talkToClient(int *clientFdPtr, char *clientStatusPtr) {
  struct sockaddr_in clientAddr;
  // First use to store received data, then also use it to store data 
  // that needs to be send to client.
  char data[MAX_DATA_SIZE];
  if (receiveData(*clientFdPtr, &clientAddr, data) <= 0) {
    printf("Client closed connection. Closing connection on server "
      "side... \n");
    closeConnection(clientFdPtr, clientStatusPtr, &clientAddr);
    return;
  }
  if (processAndReply(data, clientStatusPtr, *clientFdPtr, &clientAddr) == EXIT)
   closeConnection(clientFdPtr, clientStatusPtr, &clientAddr);
}

int main(int argc, char const *argv[]) {
  const unsigned short SERVER_PORT = loadServerConf();
  int masterSockFd = createMasterSocket(SERVER_PORT);
  int fdArray[MAX_CLIENT_SUPPORTED];
  char fdStatus[MAX_CLIENT_SUPPORTED];
  initClientSockets(fdArray, fdStatus);

  while (1) {
    fd_set fdSet;
    initFdSet(&fdSet, masterSockFd, fdArray);
    printf("Blocked on select system call...\n\n");
    select(getMaxFd(masterSockFd, fdArray)+1, &fdSet, NULL, NULL, NULL);

    if (FD_ISSET(masterSockFd, &fdSet))
      acceptConnection(masterSockFd, fdArray, fdStatus);
    else {
      for (int i=0; i<MAX_CLIENT_SUPPORTED; i++) {
        if (FD_ISSET(fdArray[i], &fdSet)) {
          talkToClient(&fdArray[i], &fdStatus[i]);
          break;
        }
      }
    }
    printNumConnectedClients(fdArray);
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