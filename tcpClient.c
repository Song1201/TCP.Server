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

testStructType clientData;
resultStructType result;

struct sockaddr_in setupDestIPv4() {
  // sockaddr_in means destination(server) address is IPv4 address
  struct sockaddr_in dest; 
  dest.sin_family = AF_INET; // Address Family is IPv4 address
  printf("Please enter server IPv4 address: ");
  char SERVER_IPV4_ADDR[IPV4_ADDR_MAX_LEN+1];
  fgets(SERVER_IPV4_ADDR,sizeof(SERVER_IPV4_ADDR),stdin);
  // Remove the '\n' from input string.
  SERVER_IPV4_ADDR[strlen(SERVER_IPV4_ADDR)-1] = '\0';  
  if (inet_pton(AF_INET,SERVER_IPV4_ADDR, &(dest.sin_addr)) != 1) {
    printf("Failed to read server IPv4 address. Exiting program... Done.\n");
    exit(1);
  }
  printf("Please enter server port number: ");
  unsigned short DEST_PORT;
  scanf("%hu",&DEST_PORT);
  dest.sin_port = DEST_PORT;
  return dest;
}

int connectToDest(const struct sockaddr_in dest) {
  int sockFd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  printf("Conneting to server... It takes at most 3 minutes.\n\n");
  if (connect(sockFd, (struct sockaddr*)&dest, ADDR_LEN) != 0) {
    printf("Connection to server failed.\nFailed server IPv4 address: %s\n"
      "Failed server port number: %hu\n",inet_ntoa(dest.sin_addr),
      dest.sin_port);
    printf("Exiting program... Done.\n");
    exit(1);
  }
  printf("Connected to server.\nConnected server IPv4 address: %s\n"
    "Connected server port number: %hu\n\n",inet_ntoa(dest.sin_addr),
    dest.sin_port);
  return sockFd;
}

static unsigned char askForAction(char *data) {
  printf("Please enter a number to choose action:\n1 login\n2 sum\n3 exit\n"
    "\n");
  unsigned char type;
  scanf("%hhu", &type);
  getchar(); // Remove the '\n' left by scanf
  memset(data, type, TYPE_FIELD_LEN);
  return type;
}

static void getLoginData(char *data) {
  printf("Please enter username: ");
  char username[MAX_USERNAME_LEN+2]; // One for '\n', the other for '\0'
  fgets(username, sizeof(username), stdin);
  username[strlen(username)-1] = '\0'; // Remove the '\n' from input string.
  strcpy(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN, username);
  printf("Please enter password: ");
  char password[MAX_PASSWORD_LEN+2];
  fgets(password, sizeof(password), stdin);
  password[strlen(password)-1] = '\0'; // Remove the '\n' from input string.
  strcpy(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN+MAX_USERNAME_LEN+1, password);
}

static void getSumData(char *data) {
  int a, b;
  printf("Enter a: ");
  scanf("%d", &a);
  printf("Enter b: ");
  scanf("%d", &b); 
  memcpy(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN, &a, sizeof(a));
  memcpy(data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN+sizeof(a), &b, sizeof(b));  
}

static int sendAndReceiveData(int sockFd, char *data, 
struct sockaddr_in *destPtr) {
  int sentRecvBytes = sendto(sockFd, data, MAX_DATA_SIZE, 0, NULL, 0);
  printf("Number of bytes sent = %d\n",sentRecvBytes);
  sentRecvBytes = recvfrom(sockFd,data,MAX_DATA_SIZE, 0, NULL, NULL);
  printf("Number of bytes received = %d\n\n",sentRecvBytes);
  return sentRecvBytes;
}

static unsigned char processReceivedData(const char *data) {
  unsigned char type = *data;
  if (type == TEXT) {
    printf("%s\n\n",data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN);
  } else if (type == SUM) {
    int sum;
    memcpy(&sum, data+TYPE_FIELD_LEN+LENGTH_FEILD_LEN, sizeof(sum));
    printf("Result = %d\n\n", sum);
  } else if (type == EXIT) printf("Server confirmed exit signal.\n");
  else printf("Server sent unknown data type.\n");
  return type;
}

int main(int argc, char const *argv[]) {
  struct sockaddr_in dest = setupDestIPv4();
  int sockFd = connectToDest(dest);
  
  while (1) {
    char data[MAX_DATA_SIZE] = "";
    unsigned char type = askForAction(data);
    
    if (type == LOGIN) getLoginData(data);
    else if (type == SUM) getSumData(data);
    else if (type == EXIT) printf("Sending exit signal to server...\n");
    else {
      printf("Invalid input.\n\n");
      continue;
    }

    if (sendAndReceiveData(sockFd, data, &dest) <= 0) {
      printf("Server closed connection.\n");
      break;
    }

    if (processReceivedData(data) == EXIT) break;
  }  
  printf("Exiting program... Done.\n");
  return 0;
}
