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
  char *digest;
  int digestLen;

	EVP_DigestFinal_ex(mdctx, digest, &digestLen);
	EVP_MD_CTX_destroy(mdctx);
  printf("Digest length: %d\n", digestLen);
  for (int i=0; i<digestLen; i++) {
    printf("%02x",(unsigned char)digest[i]);
  }
  printf("\n");

  return 0;
}

