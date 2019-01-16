#include <stdio.h> // To use printf
#include <string.h> // To use strcpy()
#include <openssl/evp.h>

int main() {
	EVP_MD_CTX *mdctx;

  mdctx = EVP_MD_CTX_create();

  EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);

  char *message = "THUNDER";

  EVP_DigestUpdate(mdctx, message, strlen(message));

	// if((*digest = (unsigned char *)OPENSSL_malloc(EVP_MD_size(EVP_sha256()))) == NULL)
	// 	handleErrors();
  unsigned char digest[EVP_MAX_MD_SIZE];
  int digestLen;

	EVP_DigestFinal_ex(mdctx, digest, &digestLen);
	EVP_MD_CTX_destroy(mdctx);
  printf("Digest length: %d\n", digestLen);
  printf("String length: %d\n", strlen(digest));
  for (int i=0; i<digestLen; i++) {
    printf("%02x", digest[i]);
  }
  printf("\n");

  // FILE *filePtr = fopen("scratch.conf", "a");
  // for (int i=0; i<digestLen; i++) {
  //   fprintf(filePtr, "%02x", digest[i]);
  // }
  // fprintf(filePtr,"\n");
  
  FILE *filePtr = fopen("scratch.conf", "r");
  unsigned char digestRead[EVP_MAX_MD_SIZE] = "";

  unsigned char digestReadString[EVP_MAX_MD_SIZE] = "";
  fscanf(filePtr, "%s", digestReadString);
  fclose(filePtr);

  printf("digest read string: %s\n",digestReadString);

  for (int i=0; i<strlen(digestReadString); i++) {
    sscanf(digestReadString+2*i, "%2x", (unsigned int*)(digestRead+i));
  }

  

  for (int i=0; i<strlen(digestRead); i++) {
    printf("%02x", digestRead[i]);
  }
  printf("\n");


  return 0;
}

