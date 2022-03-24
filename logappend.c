#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <stdbool.h>
#include <err.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>


#include "data.h"
#include "crypto.h"

char salt[] = "XXXXXXXXXXXXXXXXXXXX";
char CTUID[] = "XXXXXXXXXXXXXXXXXXXX";

typedef struct _CmdLineResult {
  // int     good; Not using good because we exit whenever input is erroneous
  int     ts;
  char*   token;
  int     token_len;
  bool    is_employee;
  bool    is_arrival;
  char*   name;
  int     name_len;
  int     roomID;
  char*   logpath;
  int     logpath_len;
  char*   batchpath;
} CmdLineResult;


CmdLineResult parse_cmdline(int argc, char *argv[], int is_batch) {
  CmdLineResult R = { -1, NULL, 0, false, false, NULL, 0, -1, NULL, 0, NULL};
  int opt = -1;
  bool invalidinput = false;
  bool EGchecked = false;
  bool ALchecked = false;
  bool nonbatch = false;
  bool kexist = false;
  //argument data
  char    *logpath = NULL;
  char    *batchpath = NULL;
  int     batchpath_len;
  char    *token;
  char    *name;

  //pick up the switches
  while ((opt = getopt(argc, argv, "T:K:E:G:ALR:B:")) != -1) {
    switch(opt) {
      case 'B':
        //batch file
        batchpath_len = strlen(optarg) + 1;
        batchpath = malloc(batchpath_len);
        memcpy(batchpath, optarg, batchpath_len);
        R.batchpath = batchpath;
	    break;  

      case 'T':
        for (int i = 0; i < strlen(optarg); i++){
                if (!isdigit(optarg[i])){
                    invalidinput = true;
                    break;
                }
            }
	    //timestamp
	    R.ts = atoi(optarg);
        if (R.ts <= 0 || R.ts > 1073741823){
            invalidinput = true;
        }
        nonbatch= true;
        break;

      case 'K':
	    //secret token
        if(kexist){
            invalidinput = true;
        }
	    R.token_len = strlen(optarg) + 1;
        token = malloc(R.token_len);
        memcpy(token, optarg, R.token_len);
        R.token = token;
        nonbatch= true;
        kexist = true;
        break;

      case 'A':
        //arrival
        R.is_arrival = true;

        // Error out on duplicate flags
        if (ALchecked == true) {
            printf("invalid\n");
            exit(255);
        }
        ALchecked = true;
        nonbatch= true;
        break; 
      
      case 'L':
        //departure
        R.is_arrival = false;

        // Error out on duplicate flags
        if (ALchecked == true) {
            printf("invalid\n");
            exit(255);
        }
        ALchecked = true;
        nonbatch= true;
        break;

      case 'E':
        //employee name
        R.name_len = strlen(optarg) + 1;
        name = malloc(R.name_len);
        memcpy(name, optarg, R.name_len);

        //check name consist of character
        for (int i = 0; i < strlen(name); i++){
            if (!isalpha(name[i])){
                invalidinput = true;
                break;
            }
        }
        R.name = name;
        R.is_employee = true;
        
        // Error out on duplicate flags
        if (EGchecked == true) {
            printf("invalid\n");
            exit(255);
        }
        EGchecked = true;
        nonbatch= true;
        break;

      case 'G':
        //guest name
        R.name_len = strlen(optarg) + 1;
        name = malloc(R.name_len);
        memcpy(name, optarg, R.name_len);

        //check name consist of character
        for (int i = 0; i < strlen(name); i++){
            if (!isalpha(name[i])){
                invalidinput = true;
                break;
            }
        }

        R.name = name;
        R.is_employee = false;
        
        // Error out on duplicate flags
        if (EGchecked == true) {
            printf("invalid\n");
            exit(255);
        }
        EGchecked = true;
        nonbatch= true;
	    break;

      case 'R':
        //room ID
        R.roomID = atoi(optarg);
        nonbatch= true;
        break;

      default:
        //unknown option, leave
        break;
    }
    if(invalidinput){
        printf("invalid\n");
        exit(255);
    }

  }
  
  //pick up the positional argument for log path
  if(optind < argc) {
    R.logpath_len = strlen(argv[optind]) + 1;
    logpath = malloc(R.logpath_len);
    memcpy(logpath, argv[optind], R.logpath_len);
    R.logpath = logpath;
  }

  // Options check for batching
  if (R.batchpath!=NULL) {
    // No other flags if doing batching
    if (nonbatch==true) {
        printf("invalid\n");
        exit(255);
    }
  }
  // Options check for non-batching
  else {
    // timestamp required
    if (R.ts==-1) {
        printf("invalid\n");
        exit(255);
    }

    // token required
    if (R.token==NULL) {
        printf("invalid\n");
        exit(255);
    }

    // log file required
    if (R.logpath==NULL) {
        printf("invalid\n");
        exit(255);
    }

    // Require both E/G and A/L
    if (!EGchecked || !ALchecked) {
        printf("invalid\n");
        exit(255);
    }
  }
  
  return R;
}

void print_cmdline(CmdLineResult R){
    printf("ts: %d\n", R.ts);
    printf("token: %s\n", R.token);
    printf("token_len: %d\n", R.token_len);
    printf("is_employee: %d\n", R.is_employee);
    printf("is_arrival: %d\n", R.is_arrival);
    printf("name: %s\n", R.name);
    printf("name_len: %d\n", R.name_len);
    printf("roomID: %d\n", R.roomID);
    printf("logpath: %s\n", R.logpath);
    printf("logpath_len: %d\n", R.logpath_len);
}

void print_logentry(LogEntry L){
    printf("ts: %d\n", L.ts);
    printf("is_employee: %d\n", L.is_employee);
    printf("is_arrival: %d\n", L.is_arrival);
    printf("name: %s\n", L.name);
    printf("name_len: %d\n", L.name_len);
    printf("roomID: %d\n", L.roomID);
}


int main(int argc, char *argv[]) {
    CmdLineResult R;
    FILE *log_fp;
    FILE *batch_fp;
    int num_read;
    char batch_line[2048];
    char command_line[2100]; 
  
    R = parse_cmdline(argc, argv, 0);

    // Do batching
    if (R.batchpath != NULL) {
        batch_fp = fopen(R.batchpath, "r");
        if (batch_fp == NULL) {
            printf("invalid\n");
            exit(255);
        }
        memset(batch_line, 0, 2048);
        while (fgets(batch_line, 2048, batch_fp)!=NULL) {
            // Make sure commands in batch file does not do batching
            if (strstr(batch_line, "-B")) {
                printf("invalid\n");
                continue;
            }
            memset(command_line, 0, 2100);
            strcpy(command_line, "./logappend ");
            strcat(command_line, batch_line);
            system(command_line);
            memset(batch_line, 0, 2048);
        }
        return 0;
    }

    // First step: check if log file exists.
    // If log file doesn't exist, create a new file and write token
    if( access( R.logpath, F_OK ) != 0 ){
        // Create new log
        log_fp = fopen(R.logpath, "w+");
        // Write token to beginning of file
        char token_len_str[4];
        serialize_int(token_len_str, R.token_len);
        fwrite(token_len_str, 1, 4, log_fp);
        fwrite(R.token, 1, R.token_len, log_fp);
        fclose(log_fp);
        encryptfile(R.logpath, R.token);
    }

    // Second step: check if token matches the one in existing log
    decryptfile(R.logpath, R.token);

    // Open log read-only
    log_fp = fopen(R.logpath, "r");
    char *buf_r;
    buf_r = malloc(4);
    num_read = fread(buf_r, 1, 4, log_fp);
    assert(num_read==4 && "4 bytes expected for token_len");
    int token_len = deserialize_int(buf_r);
    num_read = fread(buf_r, 1, token_len, log_fp);
    assert(num_read==token_len && "num_read not equal to token_len");
    // Compare tokens
    if (strcmp(buf_r, R.token) != 0) {
        printf("invalid");
        encryptfile(R.logpath, R.token);
        exit(255);
    }

    // Third step: Read through the log, search for relevant key (name+E/G), and get person's current location

    int current_location = -2;
    buf_r = malloc(4);
    num_read = fread(buf_r, 1, 4, log_fp);
    int last_ts = -1;
    while (num_read != 0) {
        assert(num_read==4 && "4 bytes expected for entry_len");
        // Deserialize one entry
        
        int entry_len = deserialize_int(buf_r);
        buf_r = realloc(buf_r, entry_len);
        memset(buf_r, 0, entry_len);
        num_read = fread(buf_r, 1, entry_len, log_fp);
        assert(num_read==entry_len && "num_read not equal to entry_len");
        LogEntry L;
        buf_to_logentry(&L, buf_r, entry_len);
        // Check match and update person's location
        if ((strcmp(R.name, L.name) == 0) && (R.is_employee == L.is_employee)) {
            if (L.is_arrival) {
                // Arriving in either gallery or room
                current_location = L.roomID;
            } else {
                if (L.roomID==-1) {
                    // Leaving gallery
                    current_location = -2;
                } else {
                    // Leaving room
                    current_location = -1;
                }
            }
        }
        // Update latest timestamp
        last_ts = L.ts;
        free(L.name);
        // Read next entry
        buf_r = realloc(buf_r, 4);
        num_read = fread(buf_r, 1, 4, log_fp);
    }

    // Fourth step: check if command line input is valid given person's current location
    // Also check for timestamp validity
    if (R.is_arrival) {
        if (R.roomID==-1 && current_location!=-2) {
            // Person already in gallery entering again
            printf("invalid\n");
            encryptfile(R.logpath, R.token);
            exit(255);
        }
        if (R.roomID>=0 && current_location!=-1) {
            printf("invalid\n");
            encryptfile(R.logpath, R.token);
            exit(255);
        }
    } else {
        if (R.roomID != current_location) {
            printf("invalid\n");
            encryptfile(R.logpath, R.token);
            exit(255);
        }
    }
    if (R.ts < last_ts) {
        printf("invalid\n");
        encryptfile(R.logpath, R.token);
        exit(255);
    }

    log_fp = fopen(R.logpath, "a");
    char *buf;
    int buf_len;
    LogEntry L;
    L.ts = R.ts;
    L.name = R.name;
    L.name_len = R.name_len;
    L.is_employee = R.is_employee;
    L.is_arrival = R.is_arrival;
    L.roomID = R.roomID;
    buf_len = logentry_to_buf(L, &buf);
    fwrite(buf, 1, buf_len, log_fp);
    fclose(log_fp);
    free(buf);
    encryptfile(R.logpath, R.token);
            
  return 0;
}
