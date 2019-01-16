#include <stdio.h> // To use printf
#include <string.h> // To use strcpy()
#include <openssl/evp.h>

int main() {
  FILE *confPtr = fopen("server.conf","r");

  unsigned short SERVER_PORT;
  fscanf(confPtr,"%hu",&SERVER_PORT);

    
  char username[50+1];
  char passwordHash[100+1];
  while (fscanf(confPtr,"%s",username) != EOF) {
    fscanf(confPtr,"%s",passwordHash);
    printf("User: ");
    printf("%s  %s\n",username, passwordHash);
  }
  fclose(confPtr);
}

