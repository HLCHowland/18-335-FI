#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <assert.h>


#include "data.h"
#include "crypto.h"

char salt[] = "XXXXXXXXXXXXXXXXXXXX";
char CTUID[] = "XXXXXXXXXXXXXXXXXXXX";

int verbose = 0;

void print_records(Record *records, unsigned int num_records) {
  unsigned int i;

  for(i = 0; i < num_records; i++) {
    dump_record(&records[i]);
    printf("----------------\n");
  }

  return;
}

struct Person {
  char* name;
  bool is_employee;
  int roomnow;
  int roomnumber;
  int *roomrecord;
  int totaltime;
  int entertime;
  SLIST_ENTRY(Person)   link;
};

SLIST_HEAD(slisthead, Person);

// Defining comparator function as per the requirement
static int strCompare(const void* a, const void* b)
{
  
    // setting up rules for comparison
    return strcmp(*(char**)a, *(char**)b);
}

bool onlyOnetrue(bool one, bool two, bool three){
  return (one && !two && !three)||(!one && two && !three)||(!one && !two && three);
}

static int intCompare(const void* a, const void* b)
{
  
    // setting up rules for comparison
    return *((int *)a) > *((int *)b);
}
  
// Function to sort the array
void sort(char* arr[], int n)
{
    // calling qsort function to sort the array
    // with the help of Comparator
    qsort(arr, n, sizeof(char*), strCompare);
}

void sorti(int arr[], int n)
{
    // calling qsort function to sort the array
    // with the help of Comparator
    qsort(arr, n, sizeof(int), intCompare);
}

void leave_action(struct slisthead *head, LogEntry L, int *timenow) {
    struct Person *current;
    *timenow = L.ts; 
    SLIST_FOREACH(current, head, link) {
        //Find the person
        if(strcmp(current->name,L.name)==0 && current->is_employee==L.is_employee){
            //Is the person in gallery?
            if(current->roomnow == -1){
                current->roomnow=-2;
                current->totaltime = current->totaltime + L.ts - current->entertime;
                current->entertime = -1;
            }
            else {
                current->roomnow=-1;
            }
        }
    }
}

void arrive_action(struct slisthead *head, LogEntry L, int *timenow) {
    struct Person *current;
    *timenow = L.ts;
    // Check if person is already in the list
    SLIST_FOREACH(current, head, link) {
        if(strcmp(current->name,L.name)==0 && current->is_employee==L.is_employee){
            current->roomnow=L.roomID;
            current->roomrecord = realloc(current->roomrecord,(current->roomnumber+2)*sizeof(int));
            current->roomrecord[current->roomnumber]=L.roomID;
            current->roomnumber++;
            if(L.roomID == -1){
              current->entertime = L.ts;
            }
            return;
        }
    }

    // Add person to the list if not found above
    current=malloc(sizeof(struct Person));
    SLIST_INSERT_HEAD(head, current, link);
    current->roomnow=L.roomID;
    current->is_employee=L.is_employee;
    current->name=(char*) malloc((strlen(L.name)+1)*sizeof(char));
    strcpy(current->name,L.name);
    current->roomnumber = 0;
    current->totaltime = 0;
    current->entertime = L.ts;
}

void print_time(struct Person *first, struct slisthead head, char *name, bool is_employee, int timenow){
    int totaltime;
    SLIST_FOREACH(first, &head, link){
        if(strcmp(first->name,name)==0 && first->is_employee==is_employee){
            if(first->entertime == -1){
              totaltime = first->totaltime;
              printf("%i\n",totaltime);
            }
            else{
              totaltime = timenow - first->entertime +first->totaltime;
              printf("%i\n",totaltime);
            }
            return;
        }
    }        
}
void print_rooms(struct Person *first, struct slisthead head, char *name, bool is_employee) {
    int i;
    SLIST_FOREACH(first, &head, link){
        if(strcmp(first->name,name)==0 && first->is_employee==is_employee){
            for(i = 0; i < first->roomnumber; i++){
                printf("%i", first->roomrecord[i]);
                if(i!=first->roomnumber-1){
                    printf(","); 
                }
                else{
                    printf("\n");
                    return;
                }
            }
        }
    }        
}

void print_summary(struct Person *first, struct slisthead head) {
    int totalroom = 0;
    int roomlist[100];
    int i, j;
    bool is_newroom = true;
    char * arrE[100];
    char * arrG[100];
    int kE = 0;
    int kG = 0;
    SLIST_FOREACH(first, &head, link){
        if (first->roomnow>=-1) {
            if (first->is_employee) {
                arrE[kE]=malloc(strlen(first->name)+1);
                strcpy(arrE[kE],first->name);
                kE++;
            } else {
                arrG[kG]=malloc(strlen(first->name)+1);
                strcpy(arrG[kG],first->name);
                kG++;
            }
    
            // Update a list of rooms
            if (first->roomnow>=0) {
                is_newroom = true;
                for(i = 0;i < totalroom; i++) {
                  if(first->roomnow==roomlist[i]) is_newroom=false;
                }
                if(is_newroom == true) {
                  roomlist[totalroom] = first->roomnow;
                  totalroom++;
                }
            }
        }
    }
    // Sort people's names and room numbers
    sort(arrE, kE);
    sort(arrG, kG);
    sorti(roomlist, totalroom);

    // Print the sorted employees
    for (i = 0; i < kE; i++) {
        printf("%s", arrE[i]);
        if (i!= kE-1) printf(",");
    }
    printf("\n");

    // Print the sorted guests
    for (i = 0; i < kG; i++) {
        printf("%s", arrG[i]);
        if (i!= kG-1) printf(",");
    }
  
    for(i = 0;i < totalroom; i++) {
        int m = 0;
        char * roomchar[100];
        printf("\n");
        printf("%i: ", roomlist[i]);

        //Store the name in the array for sorting
        SLIST_FOREACH(first, &head, link){
            if(first->roomnow == roomlist[i]) {
                roomchar[m]=malloc(strlen(first->name)+1);
                strcpy(roomchar[m],first->name);
                m++;
            }
        }
        sort(roomchar, m);
        for (j = 0; j < m; j++) {
            printf("%s", roomchar[j]);
            if(j != m-1) {
                printf(",");
            }
        }    
    }            
}

int main(int argc, char *argv[]) {
  int   opt;
  //Store parsing result
  char* token = NULL;
  int token_len_input = 0;
  char* name = NULL;
  int name_len = 0;
  bool print_S = false;
  bool print_R = false;
  bool print_T = false;
  bool is_employee = false;
  char* logpath = NULL;
  int logpath_len = 0;

  bool EGchecked = false;

// FIrst step: Parse the commnadline

  while ((opt = getopt(argc, argv, "K:SRE:G:TI")) != -1) {
    switch(opt) {
      case 'K':
        token_len_input = strlen(optarg) + 1;
        token = malloc(token_len_input);
        memcpy(token, optarg, token_len_input);
        break;
      case 'S':
        print_S = true;
        break;

      case 'R':
        print_R = true;
        break;

      case 'T':
        print_T = true; 
        break;

      case 'I':
        printf("unimplemented");
        exit(255);
        break;

      case 'E':
        //employee name
        name_len = strlen(optarg) + 1;
        name = malloc(name_len);
        memcpy(name, optarg, name_len);
        is_employee = true;
        
        // Error out on duplicate flags
        if (EGchecked == true) {
            printf("invalid\n");
            exit(255);
        }
        EGchecked = true;
        break;

      case 'G':
        //guest name
        name_len = strlen(optarg) + 1;
        name = malloc(name_len);
        memcpy(name, optarg, name_len);
        is_employee = false;
        // Error out on duplicate flags
        if (EGchecked == true) {
            printf("invalid\n");
            exit(255);
        }
        EGchecked = true;
	    break;
    }
  }

//Read the log file name
  if(optind < argc) {
    logpath_len = strlen(argv[optind]) + 1;
    logpath = malloc(logpath_len);
    memcpy(logpath, argv[optind], logpath_len);
  }
  else {
    printf("invalid\n");
    exit(255);
  }

//Double check commandline
  // Only one -S -R and -T commend
  if(!onlyOnetrue(print_S,print_R,print_T)){
    printf("invalid");
    exit(255);
  }

   

  // -R needs a guest or employee's name
  if(print_R && !EGchecked){
    printf("invalid");
    exit(255);    
  }  

  // -T needs a guest or employee's name
  if(print_T && !EGchecked){
    printf("invalid");
    exit(255);    
  }

//Second step: check if token matches the one in existing log
    FILE *log_fp;
    int num_read;
    // Open log read-only
    if( access( logpath, F_OK ) != 0 ){
      printf("invalid\n");
      exit(255);
    }
    
    // Decrypt the logfile prior to reading
    decryptfile(logpath, token);

    log_fp = fopen(logpath, "r");
    char *buf_r;
    buf_r = malloc(4);
    num_read = fread(buf_r, 1, 4, log_fp);
    assert(num_read==4 && "4 bytes expected for token_len");
    int token_len = deserialize_int(buf_r);
    num_read = fread(buf_r, 1, token_len, log_fp);
    assert(num_read==token_len && "num_read not equal to token_len");
    // Compare tokens
    if (strcmp(buf_r, token) != 0) {
        printf("invalid");
        encryptfile(logpath, token);
        exit(255);
    }

    // Third step: Read through the logentry, execute log entry one by one

    // -2=not in gallery, -1=in gallery, 0-1073741823=in room
    struct slisthead head = SLIST_HEAD_INITIALIZER(head);
    // Record the time the program has executed
    int timenow = 0;
    buf_r = realloc(buf_r, 4);
    num_read = fread(buf_r, 1, 4, log_fp);
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
        if(L.is_arrival==true){
          arrive_action(&head,L,&timenow);
          }
        else {
          leave_action(&head,L,&timenow);
          }
        free(L.name);
        // Read next entry
        buf_r = realloc(buf_r, 4);
        num_read = fread(buf_r, 1, 4, log_fp);
    }
    
    // Forth step: Print the information we want
    struct Person *first = NULL;
    fclose(log_fp);

    encryptfile(logpath, token);

    if(logpath!=NULL){
    if(print_S==true){
      print_summary(first, head);
    }
    if((print_R==true) && (name!=NULL))
      print_rooms(first, head, name, is_employee);
    if((print_T==true) && (name!=NULL))
      print_time(first, head, name, is_employee,timenow);
  }

  return 0;
}

