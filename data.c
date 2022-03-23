#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "data.h"

#define NON_VAR_LENGTH 0     //TODO change me

int compute_record_size(Record *R) {
  int len = 0;
  //TODO do stuff
  
  return len;
}

Buffer print_record(Record *R) {
  Buffer  B = {0};
 
  //TODO Code this

  return B;
}

//produce A | B
Buffer concat_buffs(Buffer *A, Buffer *B) {
  Buffer  C = {0}; 
  //TODO Code this
  return C;
}

void write_to_path(char *path, Buffer *B, unsigned char *key_data) {
  
  //TODO Code this

  return;
}

Buffer read_from_path(char *path, unsigned char *key_data) {
  Buffer  B = {0};
  
  //TODO Code this
  
  return B;
}

void dump_record(Record *R) { 
  
  //TODO Code this
  
  return;
}

int get_record(Record *R, Buffer *B) {
  unsigned int  bytesRead = 0;
 
  //TODO Code this

  return bytesRead;
}

int read_records_from_path(char *path, unsigned char *key, Record **outbuf, unsigned int *outnum) {
  *outnum = 0;
  *outbuf = NULL;  
  return 0;
}

void serialize_int(char *buf, int a) {
    buf[0] = 0xff & (a >> 24);
    buf[1] = 0xff & (a >> 16);
    buf[2] = 0xff & (a >> 8);
    buf[3] = 0xff & a;
}

int deserialize_int(char *buf) {
    return ((unsigned char)buf[0]<<24) | ((unsigned char)buf[1]<<16) | ((unsigned char)buf[2]<<8) | (unsigned char)buf[3];
}

// log format: entry_len(int) + ts(int) + name(str) + E/G(char) + A/L(char) + roomID(int)
// Total length = 4+4+name_len+1+1+4 = 14+name_len
int logentry_to_buf(LogEntry L, char **buf) {
    int entry_len = 10+L.name_len;
    *buf = calloc(entry_len+4, sizeof(char));
    serialize_int(*buf, entry_len); // entry_len
    serialize_int(*buf+4, L.ts); // timestamp
    memcpy(*buf+8, L.name, L.name_len); // name
    (*buf)[8+L.name_len] = L.is_employee ? 'E' : 'G'; // Employee/Guest
    (*buf)[9+L.name_len] = L.is_arrival ? 'A' : 'L'; // Arrive/Leave
    serialize_int(*buf+L.name_len+10, L.roomID); // roomID
    return 14 + L.name_len;
}

// Takes in a buffer pointing to the first character AFTER entry_len
// Construct and returns LogEntry in L
void buf_to_logentry(LogEntry * L, char *buf, int entry_len) {
    char * name;
    L->ts = deserialize_int(buf);
    name = calloc(entry_len-10, sizeof(char));
    memcpy(name, buf+4, entry_len-10);
    L->name = name;
    L->name_len = entry_len-10;
    L->is_employee = (buf[entry_len-6] == 'E');
    L->is_arrival = (buf[entry_len-5] == 'A');
    L->roomID = deserialize_int(buf+entry_len-4);
}

