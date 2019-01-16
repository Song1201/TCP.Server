#include "common.h"
#include <stdio.h> // To use printf
#include <string.h> // To use strlen()
#include <openssl/evp.h>

#define ADD_USER 1
#define CHANGE_PORT 2

int main() {
  while (1) {
    printf("Please enter a number to choose action:\n1 add user\n2 change server "
      "port\n3 exit\n\n");
    unsigned char type;
    scanf("%hhu", &type);
    getchar(); // Remove the '\n' left by scanf

    if (type == ADD_USER) {
      printf("Please enter username (no more than %u characters): ", 
        MAX_USERNAME_LEN);
      char username[MAX_USERNAME_LEN+2]; // One for '\n', the other for '\0'
      fgets(username, sizeof(username), stdin);
      // Remove the '\n' from input string.
      username[strlen(username)-1] = '\0';  

      printf("Please enter password (no more than %u characters): ", 
        MAX_PASSWORD_LEN);
      char password[MAX_PASSWORD_LEN+2]; // One for '\n', the other for '\0'
      fgets(password, sizeof(password), stdin);
      // Remove the '\n' from input string.
      password[strlen(password)-1] = '\0';

      printf("Recorded username: %s\nRecorded password: %s\n", username, 
        password);

      FILE *filePtr = fopen("server1.conf", "a");
      fprintf(filePtr, "%s ", username);

      // Hash the password
      EVP_MD_CTX *hasher;

      hasher = EVP_MD_CTX_create();

      EVP_DigestInit_ex(hasher, EVP_sha256(), NULL);

      EVP_DigestUpdate(hasher, password, strlen(password));

      unsigned char pwDigest[EVP_MAX_MD_SIZE];
      unsigned int digestLen;
      EVP_DigestFinal_ex(hasher, pwDigest, &digestLen);
      EVP_MD_CTX_destroy(hasher);
      // printf("Digest length: %d\n", digestLen);
      // printf("String length: %d\n", strlen(digest));
      // for (int i=0; i<digestLen; i++) {
      //   printf("%02x", digest[i]);
      // }
      // printf("\n");

      // FILE *filePtr = fopen("scratch.conf", "a");
      for (int i=0; i<digestLen; i++) {
        fprintf(filePtr, "%02x", pwDigest[i]);
      }
      fprintf(filePtr,"\n");
      fclose(filePtr);
      
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


    }
  }








  return 0;
}

