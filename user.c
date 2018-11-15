//USER

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <fcntl.h>


// shared memory key and size of shared memory.. size of an int(4 bytes)
#define SHMKEYA 321800
#define SHMKEYB 321801
#define BUFF_SZ	sizeof ( int )
//----------------------------------------------------

//struct for clock
typedef struct {
    unsigned int seconds;
    unsigned int nanoSeconds;
} clockTime;
//---------------------
typedef struct {
    pid_t pids[18];
    int pidJob[18];
    int nanosRequest[18];
    
// tables for bakers algorithm
    int rescources[20];
    int max[18][20];
    int allocated[18][20];
    int request[18][20];
} resourceMemory;
//---------------------

//struct for message queues
struct mesg_buffer {
    long mesg_type;
    char mesg_text[100];
} message;
//---------------------



//global variables
int clockSHMID;//shared memory id
clockTime *clockShmPtr; //pointer to data struct
int resourceSHMID;//resource
resourceMemory *resourcePointer;
int count = 0;
int pidHolder[18] = {};
int randomClockTime[18] = {};

//message queue
key_t key;
int msgid;

//-------------------------------

//function headers
void signalHandler(int sig);
void cleanUp();
void shareMemory();
void messageQueueConfig();

//----------------------------------------------------
int main(int argc, char* argv[]) 
{
int incrementClock;




  printf("USER");
return 0;//main return
}//main

//----------------------------------------------------

void shareMemory()
{
    //shared mem for sysClock
    clockSHMID = shmget(SHMKEYA, sizeof(clockTime), IPC_CREAT|0777);
    if(clockSHMID < 0)
    {
        printf("ERROR: OSS shmget \n");
        exit(EXIT_FAILURE);
    }
    clockShmPtr = shmat(clockSHMID, NULL, 0);
    if(clockShmPtr < 0){
        printf("Error: OSS shmat\n");
        exit(EXIT_FAILURE);
    }

    //shared mem for Rescource Descriptor
    resourceSHMID = shmget(SHMKEYB, sizeof(resourceMemory), IPC_CREAT|0777);
    if(resourceSHMID < 0)
    {
        printf("ERROR: OSS shmget resource\n");
        exit(EXIT_FAILURE);
    }
    resourcePointer = shmat(resourceSHMID, NULL, 0);
    if(resourcePointer < 0){
        printf("ERROR: OSS shmget resource\n");
        exit(EXIT_FAILURE);
    }
}
//setup message queue function
void messageQueueConfig()
{
    key = ftok("oss.c", 10);

    msgid = msgget(key, 0666 | IPC_CREAT);
}

//------------------------
void signalHandler(int sig)
{
  if( sig == SIGINT)
  {
    printf("OSS: CTRL+C Caught: Memory Clean Up");
    cleanUp();
  }
}

//memory cleanup
void cleanUp()
{
  printf("\nClean up started....\n");
  //shmctl(shmidA, IPC_RMID, NULL);
//  shmctl(shmidB, IPC_RMID, NULL);
//  free(pcpids);
 // sem_close(semaphore);
  
}