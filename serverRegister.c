#include "common.h"
#include <stdio.h> // To use printf
#include <string.h> // To use strlen()
#include <openssl/evp.h>
#include "server.h"

#define ADD_USER 1
#define REGISTER_EXIT 2

static unsigned char askForAction() {
  printf("Please enter a number to choose action:\n1 add user\n2 exit\n\n");
  unsigned char action;
  scanf("%hhu", &action);
  getchar(); // Remove the '\n' left by scanf
  return action;
}

static void hashPassword(char *password, unsigned char *pwDigest) {
  // Hash the password
  EVP_MD_CTX *hasher;
  hasher = EVP_MD_CTX_create();
  EVP_DigestInit_ex(hasher, EVP_sha256(), NULL);
  EVP_DigestUpdate(hasher, password, strlen(password));
  EVP_DigestFinal_ex(hasher, pwDigest, NULL);
  EVP_MD_CTX_destroy(hasher);
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

static void registerUser(char *username, char *password) {
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

      // FILE *filePtr = fopen("scratch.conf", "r");
      // unsigned char digestRead[EVP_MAX_MD_SIZE] = "";

      // unsigned char digestReadString[EVP_MAX_MD_SIZE] = "";
      // fscanf(filePtr, "%s", digestReadString);
      // fclose(filePtr);

      // printf("digest read string: %s\n",digestReadString);

      // for (int i=0; i<strlen(digestReadString); i++) {
      //   sscanf(digestReadString+2*i, "%2x", (unsigned int*)(digestRead+i));
      // }

      

      // for (int i=0; i<strlen(digestRead); i++) {
      //   printf("%02x", digestRead[i]);
      // }
      // printf("\n");

      // printf("Digest length: %d\n", digestLen);
      // printf("String length: %d\n", strlen(digest));
      // for (int i=0; i<digestLen; i++) {
      //   printf("%02x", digest[i]);
      // }
      // printf("\n");

      // FILE *filePtr = fopen("scratch.conf", "a");