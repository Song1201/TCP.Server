typedef struct _testStruct {
  unsigned int a;
  unsigned int b;
} testStructType;

typedef struct _resultStruct {
  unsigned int c;
} resultStructType;

typedef struct _typeLengthValue {
  char type;
  char length;
  char *value;
} tlv;

#define LOGIN 1
#define MAX_USERNAME_LEN 20
#define MAX_PASSWORD_LEN 20
#define MAX_DATA_SIZE 1024