#ifndef _BUFFR_H
#define _BUFFR_H
#include <stdbool.h>
typedef struct _Buffer {
  unsigned char *Buf;
  unsigned long Length;
} Buffer;

typedef enum _GuestType {
  Employee,
  Guest
} GuestType;

typedef enum _ActionType {
  Arrived,
  Left
} ActionType;

typedef struct _Record {
  //put some things here
} Record;

// Only included useful information for each log entry
typedef struct _LogEntry {
  int     ts;
  char*   name;
  int     name_len;
  bool    is_employee;
  bool    is_arrival;
  int     roomID;
} LogEntry;


Buffer read_from_path(char *path, unsigned char *key);
void write_to_path(char *path, Buffer *B, unsigned char *key);
Buffer concat_buffs(Buffer *A, Buffer *B);
Buffer print_record(Record *R);
void dump_record(Record *R);

int read_records_from_path(char *path, unsigned char *key, Record **, unsigned int *);

// Our helper functions
void serialize_int(char *buf, int a);
int deserialize_int(char *buf);
int logentry_to_buf(LogEntry L, char **buf);
void buf_to_logentry(LogEntry * L, char *buf, int entry_len);

#endif
