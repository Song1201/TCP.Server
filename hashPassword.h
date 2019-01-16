// password sha256 digest length. 256/8 = 32
#define PW_DIGEST_LEN 32 

void hashPassword(const char *password, unsigned char *pwDigest);