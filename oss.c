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

#define MAX_FORKS 18
#define MAX_RAND 5
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
int count = 0; //lines of logfile
int s=10; //default s value is 10
int requestTime = 0;
int pidHolder[18] = {};
int randomClockTime[18] = {};
int blockedQueue[18] = {};
void writeResultsToLog();

//message queue
key_t key;
int msgid;

//-------------------------------
//function headers
void signalHandler(int sig);
void cleanUp();
void messageQueueConfig();
void shareMemory();
void initTable();
void ossClock();
void createProcess(int pidHolder[]);
void checkMsgQ();
void logAllocatedMatrix();
void processJob(int);
void logProcDetected(int procNumber, int reqNum);
void logBlocked(int procNumber, int reqNum);
void logAllocated(int procNumber, int reqNum);


//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  int input; //user input 

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
 //---------------------------------------------------------------
 
   //set up shared memory
  shareMemory();
  
  //setup message Qs
  messageQueueConfig();
 
//signal and alarm handlers
signal(SIGINT,signalHandler);
alarm(2); //alarm after 2 seconds
//------------------------------


//init the resource table
initTables();
 
//increment clock
while(1)
{
  //to increment clock by 1.5s
  ossClock();
  createProcess(pidHolder); 
  checkMsgQ();
  
  //print table
  logAllocatedMatrix();
   writeResultsToLog();
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
//-------------------------------------------------------
//check message que function
void checkMsgQ(){
    int pidPass;
    msgrcv(msgid, &message, sizeof(message), 1, IPC_NOWAIT);

    if(message.mesg_text[0] != '0') {
        pidPass = atoi(message.mesg_text);
        processJob(pidPass);
    }

    strcpy(message.mesg_text, "0");
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
shmdt(clockShmPtr);
  
}
//----------------------------------------------

void ossClock(){

    int clockIncrement = 150000; // increment clock by 150000 1.5s
    int rollover;

    if ((clockShmPtr->nanoSeconds + clockIncrement) > 999999999){
        rollover = (clockShmPtr->nanoSeconds + clockIncrement) - 999999999;
        clockShmPtr->seconds += 1;
        clockShmPtr->nanoSeconds = rollover;
    } else {
        clockShmPtr->nanoSeconds += clockIncrement;
    }
    usleep(200);
}

void createProcess(int pidHolder[]){
    int i, k;
    for(i = 0; i < MAX_FORKS; i++){
        if(pidHolder[i] == 0) {

            for(k = 0; k < 20; k++){
                resourcePointer->request[i][k] = (rand() % MAX_RAND) + 1;
            }
            int randPercent = (rand() % 100) + 1;

            if(randPercent >= 91){          
                resourcePointer->pidJob[i] = 0;
            }else if (randPercent >= 60){ 
                resourcePointer->pidJob[i] = 1;
            }else{                       
                resourcePointer->pidJob[i] = 2;
            }
            randomClockTime[i] = (rand() % 500000000) + 1500000;

            char stashbox[10];
            sprintf(stashbox, "%d", randomClockTime[i]);

            ////fork process
            if ((pidHolder[i] = fork()) == 0) {
                execl("./oss", stashbox, NULL);
            }
        }
    }
}
//---------------------------
void logAllocatedMatrix(){

    int lines;
    int ch = 0;

    FILE *fp = fopen("log.txt", "a+");

    while(!feof(fp))
    {
        ch = fgetc(fp);
        if(ch == '\n')
        {
            lines++;
        }
    }
    if(lines % 20 == 0) {

    fprintf(fp, "\n");

    fprintf(fp, "Allocate Table\n");
    fprintf(fp, "-----------------------------------------------------------\n");
    fprintf(fp, "     \n");
    int i, k;
    for(k = 0; k < 20; k++){
        fprintf(fp, "R%02i ", k);
    }

    fprintf(fp, "\n");

    for(i = 0; i < 18; i++){
        fprintf(fp, "P%02i:", i);
        for(k = 0; k < 20; k++){
            fprintf(fp, "%4d", resourcePointer->allocated[i][k]);
            count++;
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "\n");
    }

    fclose(fp);

}

//----------------------------------------------------------------------------
void initTables(){

    // init clock to random
    time_t t;
    srand((unsigned) time(&t));

    int randomNum;

    // for max table
    int i, k;
    for(i = 0; i < 18; i++){
        for(k = 0; k < 20; k++){
            randomNum = (rand() % MAX_RAND) + 1;
            resourcePointer->max[i][k] = randomNum;
        }
    }

    // for initial rescources
    for(i = 0; i < 20; i++){
        randomNum = (rand() % 10) + 1;
        resourcePointer->rescources[i] = randomNum;
    }
}
//-------------------------------------------------------------------------
void processJob(int pid){

    int jobNumber;
    int procNumber;
    int rescourceRequestNumber;

    //get job number for pid
    int i;
    for(i = 0; i < 18; i++){
        if(pidHolder[i] == pid){
            jobNumber = resourcePointer->pidJob[i];
            procNumber = i;
            rescourceRequestNumber = (rand() % 20);
            // write to log file
            logProcDetected(procNumber, rescourceRequestNumber);
        }
    }
    if(jobNumber == 1 || jobNumber == 2)
    { 
        if(resourcePointer->request[procNumber][rescourceRequestNumber] <= resourcePointer->rescources[rescourceRequestNumber])
        {
            resourcePointer->allocated[procNumber][rescourceRequestNumber] = resourcePointer->request[procNumber][rescourceRequestNumber];
            //update rescources
            resourcePointer->rescources[rescourceRequestNumber] -= resourcePointer->request[procNumber][rescourceRequestNumber];
            logAllocated(procNumber, rescourceRequestNumber);
        } 
        else 
        {
            //assign to blocked queue
            int i;
            int posted = 0;
            for(i = 0; i < 18; i++){
                if(blockedQueue[i] == 0){
                    blockedQueue[i] = pid;
                    posted = 1;
                    //write to log
                    logBlocked(procNumber, rescourceRequestNumber);
                }
                if(posted == 1){
                    i = 18;
                }
            }
        }
    } 
    else if(jobNumber == 0) 
    {            
       requestTime = 1;
        pidHolder[procNumber] = 0;
    }
}
//-------------------------------------------------------
void logProcDetected(int procNumber, int reqNum){

    FILE *fp = fopen("log.txt", "a+");
    fprintf(fp, "OSS: detected Process P%d requesting R%d at time %d:%d\n",
            procNumber, reqNum, clockShmPtr->seconds, clockShmPtr->nanoSeconds);
    fclose(fp);

}
//-------------------------------------------------------------------------
void logAllocated(int procNumber, int reqNum){

    FILE *fp = fopen("log.txt", "a+");
    fprintf(fp, "OSS: granted P%d request R%d at time %d:%d\n",
            procNumber, reqNum, clockShmPtr->seconds, clockShmPtr->nanoSeconds);
    fclose(fp);

}
void logBlocked(int procNumber, int reqNum){

    FILE *fp = fopen("log.txt", "a+");

    fprintf(fp, "OS blocking P%d for requesting R%d at time %d:%d\n",
            procNumber, reqNum, clockShmPtr->seconds, clockShmPtr->nanoSeconds);

    fclose(fp);

}
//--------------------------------------------------------------
void writeResultsToLog(){

    FILE *fp = fopen("log.txt", "a+");

    int i, k;

    // init max table
    fprintf(fp, "MAX TABLE \n");
        fprintf(fp, "-----------------------------------------------------------\n");
    fprintf(fp, "     ");
    for(k = 0; k < 20; k++){
        fprintf(fp, "R%02i ", k);
    }

    fprintf(fp, "\n");

    for(i = 0; i < 18; i++){
        fprintf(fp, "P%02i:", i);
        for(k = 0; k < 20; k++){
            fprintf(fp, "%4d", resourcePointer->max[i][k]);
            count++;
        }
        fprintf(fp, "\n");
    }

    // for initial rescources
    fprintf(fp, "\nRESCOURCES\n");
        fprintf(fp, "-----------------------------------------------------------\n");
    fprintf(fp, "     ");
    for(k = 0; k < 20; k++){
        fprintf(fp, "R%02i ", k);
    }

    fprintf(fp, "\n    ");

    for(k = 0; k < 20; k++){
        fprintf(fp, "%4d", resourcePointer->rescources[k]);
    }

    fprintf(fp, "\n");

    // init max table
    fprintf(fp, "REQUEST TABLE \n");
    fprintf(fp, "-----------------------------------------------------------\n");
    fprintf(fp, "     ");
    for(k = 0; k < 20; k++){
        fprintf(fp, "R%02i ", k);
    }

    fprintf(fp, "\n");

    for(i = 0; i < 18; i++){
        fprintf(fp, "P%02i:", i);
        for(k = 0; k < 20; k++){
            fprintf(fp, "%4d", resourcePointer->request[i][k]);
            count++;
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "\n");

    fprintf(fp, "JOBS\n");
        fprintf(fp, "-----------------------------------------------------------\n");
    for(i = 0; i < MAX_FORKS; i++){
        fprintf(fp, "%d    ", resourcePointer->pidJob[i]);
    }

    fprintf(fp, "\n");

    fprintf(fp, "TIME INTERVALS\n");
        fprintf(fp, "-----------------------------------------------------------\n");
    for(i = 0; i < MAX_FORKS; i++){
        fprintf(fp, "%d    ", randomClockTime[i]);
    }

    fprintf(fp, "\n");

    // init max table
    fprintf(fp, "ALLOCATED\n");
        fprintf(fp, "-----------------------------------------------------------\n");
    fprintf(fp, "     ");
    for(k = 0; k < 20; k++){
        fprintf(fp, "R%02i ", k);
    }

    fprintf(fp, "\n");

    for(i = 0; i < 18; i++){
        fprintf(fp, "P%02i:", i);
        for(k = 0; k < 20; k++){
            fprintf(fp, "%4d", resourcePointer->allocated[i][k]);
            count++;
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "\n");

    fprintf(fp, "BLOCKED QUEUE\n");
        fprintf(fp, "-----------------------------------------------------------\n");
    for(i = 0; i < MAX_FORKS; i++){
        fprintf(fp, "%d    ", blockedQueue[i]);
    }



    fclose(fp);
}