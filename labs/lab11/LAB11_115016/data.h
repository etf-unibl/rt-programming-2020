#ifndef DATA_H
#define DATA_H

#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define BROJ_TASKOVA 3

/* Struktura vezana za task!*/
typedef struct{
	struct itimerspec t;
    timer_t timer;
	sigset_t sigset;
	uint64_t offs;
	uint32_t period;
	void (*task_job)();	// Pokazivac na funkciju JOB programske niti!
	int prioritet; 
}STRUKTURA_TASK; 


/* Enumeracija vremena aktivacije! */
enum VREMENA {   OFFSET_0 = 1000000,
				 OFFSET_1 = 1000000,	    
				 OFFSET_2 = 1000000,	    
				 PERIOD_0 = 40000,			//40ms
				 PERIOD_1 = 80000 ,		    //80ms!
				 PERIOD_2 = 120000		    //120ms!
				};

/* Task job prvog taska! */
// periodicni zadatak na 40 milisekundi
void task_job_1(void)
{
  int i,j;
 
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
  printf("\n");
}

/* Task job drugog taska! */
// periodicni zadatak na 80 milisekundi
void task_job_2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
  printf("\n");
}

/* Task job treceg taska! */
// periodicni zadatak na 120 milisekundi
void task_job_3(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<100000; j++) ;
    printf("3");
    fflush(stdout);
  }
  printf("\n");
}

#endif
