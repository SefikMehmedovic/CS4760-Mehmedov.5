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
#define BUFF_SZ	sizeof ( int 
//----------------------------------------------------
//struct for clock
typedef struct {
    unsigned int seconds;
    unsigned int nanoSeconds;
} clockTime;



//function headers
void signalHandler(int sig);
void cleanUp();

int main(int argc, char* argv[])
{
  int input, s;
  
  
  
  
  //command line... should make this a function...//todo make function command parsing//
	while ((input = getopt(argc, argv, "hs:l:")) != -1){
		switch(input){
			
			case 'h':
				printf("Usage:\n");
				printf("./oss  runs the program with default settings.\n");
				printf("./oss -l fileName changes fileName\n");
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
			case 'l':
				//fileRename = optarg;
				printf("\nRename File\n");
				
			break;		
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
