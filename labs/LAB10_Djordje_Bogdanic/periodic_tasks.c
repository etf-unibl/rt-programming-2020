#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>


//ova struktura sluzi za u tredovsku funkciju prenesem parametre za za tajmer offs i period za svaki od taskova

typedef struct {
 
  uint64_t offs;
  int period;
}intervals;


intervals inter1={.offs=1000000,.period=60000};
intervals inter2={.offs=1000000,.period=80000};




static sigset_t sigset;
static void wait_next_activation(void)
{
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
   return timer_settime(timer, 0 /*TIMER_ABSTIME*/, &t, NULL);//izbacujemo
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




//thread_job1 i thread_job2 su periodicni taskovi koji se zivrsavaju svaki na svom tredu 


void *thread_job1(void *arg){

    //inicijalizacija
    intervals int1=*(intervals*)arg;
    int res;
    //startovanje tajmera
    res = start_periodic_timer(int1.offs,int1.period);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }
 

    
    while(1){

    //job_body
    task1();  
    //cekanje na sledecu aktivaciju 
    wait_next_activation();

    
  

   }
}




void *thread_job2(void *arg){

   intervals int2= *(intervals*)arg;
    int res;

    res = start_periodic_timer(int2.offs,int2.period);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }

    while(1){
    
    task2();
    wait_next_activation();
    

   }
}


















int main(){

pthread_t thread1;
pthread_t thread2;



sigset_t sig_alrm;
sigemptyset(&sig_alrm);
sigaddset(&sig_alrm,SIGALRM);
sigprocmask(SIG_BLOCK,&sig_alrm,NULL);



pthread_create(&thread1,NULL,thread_job1,&inter1);
pthread_create(&thread2,NULL,thread_job2,&inter2);



pthread_join(thread1,NULL);
pthread_join(thread2,NULL);


}







