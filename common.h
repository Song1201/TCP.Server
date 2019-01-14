#define LOGIN 1
#define SUM 2
#define EXIT 3
#define TYPE_FIELD_LEN 1 // Using 1 byte unsigned char
#define LENGTH_FEILD_LEN 1 // Using 1 byte unsigned char
#define MAX_USERNAME_LEN 20
#define MAX_PASSWORD_LEN 20
#define MAX_DATA_SIZE 1024

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