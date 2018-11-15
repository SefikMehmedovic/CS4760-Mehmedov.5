//OSS
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
//#define BUFF_SZ	sizeof ( int )
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
int resourceSHMID;//resource shared memory id
resourceMemory *resourcePointer;
int count = 0;
int s=10; //default s value is 10
int pidHolder[18] = {};
int randomClockTime[18] = {};
int blockedQueue[18] = {};

//message queue
key_t key;
int msgid;

//-------------------------------
//function headers
void signalHandler(int sig);
void cleanUp();
void messageQueueConfig();
void shareMemory();

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  int input;
  
  
  
  
  //command line... should make this a function...//todo make function command parsing//
	while ((input = getopt(argc, argv, "hs:")) != -1){
		switch(input){
			
			case 'h':
				printf("Usage:\n");
				printf("./oss  runs the program with default settings.\n");
				printf("./oss -s x	runs the amount of processes default is 10\n");
				printf("./oss -h  displays help info\n");
			exit(0);
			break;
			
			//user processes 
			case 's':
				 s = atoi(optarg);
				
        //user process >19 or <1 checking
				if(s > 19 || s < 1){
					fprintf(stderr,"Invalid number of user processes (%s) entered. Minimum is 1. Maximum is 19. Integers only.\n", s);
					exit(1);
				}
			break;
			// filename config switch
	
			// handle missing required arguments and unknown arguments
			case'?':
				if(optopt == 's' || optopt == 'f'){
					fprintf(stderr,"Switch -%c requires an additional argument.\n", optopt);
				}
				else if(isprint(optopt)){
					fprintf(stderr,"Invalid option -%c.\n", optopt);
					fprintf(stderr,"Use ./oss -h to display usage information.\n");
				}
			exit(1);
		}
	}
 
 return 0;//return main
}// main

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

//shared memory function
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
void messageQueueConfig(){
    key = ftok("oss.c", 10);

    msgid = msgget(key, 0666 | IPC_CREAT);
}


//------signal handler for CTRL+C
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
