#include <openssl/evp.h> // To hash
#include <string.h> // To use strlen()

void hashPassword(const char *password, unsigned char *pwDigest) {
  EVP_MD_CTX *hasher;
  hasher = EVP_MD_CTX_create();
  EVP_DigestInit_ex(hasher, EVP_sha256(), NULL);
  EVP_DigestUpdate(hasher, password, strlen(password));
  EVP_DigestFinal_ex(hasher, pwDigest, NULL);
  EVP_MD_CTX_destroy(hasher);
}