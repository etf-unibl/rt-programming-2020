//arm-linux-gnueabihf-gcc periodic_task_1.c -std=gnu99 -Wall -o periodic_task_1
//gcc -Wall periodic_task_posix_timer.c -lrt -o naziv_izlaznog_fajla
// -lrt linkuje librt biblioteku u kojoj se nalaze timer_create i timer_settime
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>


void task1(void)
{
  int i,j;
 
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
    //printf("\n");
//fflush(stdout);
}
void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
 // printf("\n");
//fflush(stdout);
}

static sigset_t sigset;
static void wait_next_activation(void)
{
//printf("wait next aktivaciju \n");
    int dummy;

    sigwait(&sigset, &dummy);
}
int start_periodic_timer(uint64_t offs, int period)
{
    struct itimerspec t;
    struct sigevent sigev;
    timer_t timer;
    const int signal = SIGALRM;
    int res;

    t.it_value.tv_sec = offs / 1000000;
    t.it_value.tv_nsec = (offs % 1000000) * 1000;
    t.it_interval.tv_sec = period / 1000000;
    t.it_interval.tv_nsec = (period % 1000000) * 1000;
	
	// ovaj dio ostaje isti
    sigemptyset(&sigset);
    sigaddset(&sigset, signal);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

	// alociranje i popunjavanje sigevent strukture
    memset(&sigev, 0, sizeof(struct sigevent));
    sigev.sigev_notify = SIGEV_SIGNAL; // kada vrijeme istekne 
									   // okinuce se signal, jer se ostatak
									   // koda oslanja na signale
    sigev.sigev_signo = signal;	// koji signal? pa SIGALRM, naravno
	
	// kreiranje tajmera
    res = timer_create(CLOCK_MONOTONIC, &sigev, &timer);
    if (res < 0) {
        perror("Timer Create");

	exit(-1);
    }
	// pokretanje tajmera 
    return timer_settime(timer, 0 /*TIMER_ABSTIME*/, &t, NULL);
}



void* thread_task1(void *arg)
{
     
    int res;
    res = start_periodic_timer(100,60000);  // unos u mikrosekundama
    if (res < 0) {
        perror("Start Periodic Timer for task1");

        return -1;
    }
    while(1)
    {
    //printf("ceka aktivaciju TASK___1\n");
        task1();
        wait_next_activation();
    }
    return NULL;
}

void* thread_task2(void *arg)
{
  
    int res;

    res = start_periodic_timer(100,80000);
    if (res < 0) {
        perror("Start Periodic Timer for task2");

        return -1;
    }
    while(1)
    {
//printf("ceka aktivaciju TASK___2\n");
        task2();
        wait_next_activation();

    }
    return NULL;
}
int main(int argc, char *argv[])
{
    int error;
    pthread_t thread[2];
   
    sigset_t sig_alarm;
    sigemptyset(&sig_alarm);
    sigaddset(&sig_alarm,SIGALRM);
    sigprocmask(SIG_BLOCK,&sig_alarm,NULL);
    printf(" Pres Enter to stop program\n");
  
    error=pthread_create(&thread[0],NULL,thread_task1,0);
    if(error)
    {   
        printf("Error, Thread can't create!\n");
        return -1;
    }
    error=pthread_create(&thread[1],NULL,thread_task2,0);
    if(error)
    {   
        printf("Error, Thread can't create!\n");
        return -1;
    }

    getchar();
    return 0;
}
