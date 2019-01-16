#include "common.h" // To use MAX_USERNAME_LEN
#include <stdio.h> // To use printf
#include <string.h> // To use strlen()
#include "hashPassword.h"

#define ADD_USER 1
#define REGISTER_EXIT 2

static unsigned char askForAction() {
  printf("Please enter a number to choose action:\n1 add user\n2 exit\n\n");
  unsigned char action;
  scanf("%hhu", &action);
  getchar(); // Remove the '\n' left by scanf
  return action;
}

static void askForUserInfo(char *username, char *password) {
  printf("Please enter username (no more than %u characters): ", 
    MAX_USERNAME_LEN);
  fgets(username, MAX_USERNAME_LEN+2, stdin);
  // Remove the '\n' from input string.
  username[strlen(username)-1] = '\0';  
  printf("Please enter password (no more than %u characters): ", 
    MAX_PASSWORD_LEN);      
  fgets(password, MAX_PASSWORD_LEN+2, stdin);
  // Remove the '\n' from input string.
  password[strlen(password)-1] = '\0';
}

static void registerUser(const char *username, const char *password) {
  FILE *filePtr = fopen("server.conf", "a");
  fprintf(filePtr, "%s ", username);

  unsigned char pwDigest[PW_DIGEST_LEN];
  hashPassword(password, pwDigest);

  for (int i=0; i<PW_DIGEST_LEN; i++) {
    fprintf(filePtr, "%02x", pwDigest[i]);
  }
  fprintf(filePtr,"\n");
  fclose(filePtr);

  printf("\nSuccessfully added username: %s  password: %s\n\n", username, 
    password);
}

int main() {
  while (1) {
    unsigned char action = askForAction();
    if (action == ADD_USER) {
      char username[MAX_USERNAME_LEN+2]; // One for '\n', the other for '\0'
      char password[MAX_PASSWORD_LEN+2]; // One for '\n', the other for '\0'
      askForUserInfo(username, password);
      registerUser(username, password);
    } else if (action == REGISTER_EXIT) break;
    else printf("Invalid input.\n\n");
  }
  return 0;
}