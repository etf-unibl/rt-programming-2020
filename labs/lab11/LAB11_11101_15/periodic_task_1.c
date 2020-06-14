#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include "getch.h"
//arm-linux-gnueabihf-gcc periodic_task_1.c -std=gnu99 -Wall -o periodic_task_1
//gcc periodic_task_1.c -std=gnu99 -Wall -o periodic_task_1

#define wait_next_activation pause
static void sighand(int s)
{
	
}
int start_periodic_timer(uint64_t offs, int period)
{
    struct itimerval t;

    t.it_value.tv_sec = offs / 1000000;
    t.it_value.tv_usec = offs % 1000000;
    t.it_interval.tv_sec = period / 1000000;
    t.it_interval.tv_usec = period % 1000000;

    signal(SIGALRM, sighand);

    return setitimer(ITIMER_REAL, &t, NULL);
}

void task1(void)
{
  int i,j;
 
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
}

void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
}
void task3(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<100000; j++) ;
    printf("3");
    fflush(stdout);
  }
}

int main(int argc, char *argv[])
{

   pid_t child1,child2,child3;
  
    child1 = fork();
    if (child1 < 0) {
        perror("Fork");
        return -1;
    }
  
   
    if (child1 == 0) {
		// child code
		int pmin = sched_get_priority_min(SCHED_FIFO);
		struct sched_param param; 
		param.sched_priority = pmin + 12; 
		sched_setscheduler(0, SCHED_FIFO, &param);
		int res1 = start_periodic_timer(2000000, 40000);
		if (res1 < 0) {
        		perror("Start Periodic Timer");
			return -1;
    		}
		while(1) {
    		wait_next_activation();
			task1();
		}
   }

   child2 = fork();
   if (child2 < 0) {
       perror("Fork");
       return -1;
    }
	
    if (child2 == 0) {
		// child code
		int pmin = sched_get_priority_min(SCHED_FIFO);
		struct sched_param param; 
		param.sched_priority = pmin + 11; 
		sched_setscheduler(0, SCHED_FIFO, &param);
		int res2 = start_periodic_timer(2000000, 80000);
		if (res2 < 0) {
        		perror("Start Periodic Timer");
			return -1;
    		}

        while(1) {
		wait_next_activation();
			task2();
    	}
		// child code
    }

	child3 = fork();
    if (child3 < 0) {
        perror("Fork");
        return -1;
    }

    if (child3 == 0) {
		// child code
		int pmin = sched_get_priority_min(SCHED_FIFO);
		struct sched_param param; 
		param.sched_priority = pmin + 10; 
		sched_setscheduler(0, SCHED_FIFO, &param);
		int res3 = start_periodic_timer(2000000, 120000);
		if (res3 < 0) {
        		perror("Start Periodic Timer");
			return -1;
    		}

        while(1) {
		wait_next_activation();
			task3();
		}
		// child code
    }
    //nezgodno je ispratiti ubijanje djece procesa jer se program nasilno prekida(zato sto je periodican do beskonacnosti)
	//zato sam realizovao da se program gasi ako se pritisne slovo q
 	//takodje moguce je realizovati i da roditeljski proces ceka da djeca zavrse, ali bi u ovom slucaju djecu procese trebalo ubijati iz terminala.
	char quit='r';
    changemode(1);
	while(quit!='q')
	{
  		while ( !kbhit() )
  		{
    		
  		}
 
  		quit = getchar();
 		if(quit=='q')
		{
			if (kill (child1, SIGKILL) == -1) {
       			printf ("kill of child failed"); 
       			return -1;
			}
    		if (kill (child2, SIGKILL) == -1) {
       			printf ("kill of child failed"); 
				return -1;
			}
			if (kill (child3, SIGKILL) == -1) {
			   printf ("kill of child failed"); 
			   return -1;
			}
			
			
		}
		usleep(120*1000);
  		
 	}
	changemode(0);
   	
    return 0;
}

   

