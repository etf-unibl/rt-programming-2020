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
#include <semaphore.h>
#include <pthread.h>


static sigset_t sigset;
static sem_t kraj,t1,t2;

void task1(void )
{
  int i,j;
  for (i=0; i<4; i++) 
  {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
}

void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) 
  {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
}

/*ceka SIGALRM od tajmera, u zavisnosti koji put se tajmer aktivira postuju se semafori niti koje trebaju tad da se izvrse. 
 * svaki sesti (osmi) put se signalizira tasku 1 (2), jer tajmer okida na svakih 10ms*/
static void wait_next_activation()
{
    int dummy;
    static int brojac;
	sigwait(&sigset, &dummy);
    brojac++;
    if((brojac%6)==0)
		sem_post(&t1);
	if((brojac%8)==0)
		sem_post(&t2);
    
}
int start_periodic_timer(uint64_t offs, int period)
{
    struct itimerspec t;
    struct sigevent sigev;
    timer_t timer;
    const int signal = SIGALRM;
    int res;

    t.it_value.tv_sec = offs / 1000;
    t.it_value.tv_nsec = (offs - (offs / 1000)*1000) * 1000*1000;
    t.it_interval.tv_sec = period / 1000;
    t.it_interval.tv_nsec = ( period - (period / 1000)*1000) *1000*1000;
	
    sigemptyset(&sigset);//ispraznimo sigset jer je inicjalno dobio nesto
    sigaddset(&sigset, signal);//dodamo signale koje mi trebamo
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

/*funkcija za nit koja predstavlja task 1, ceka da joj se signalizira da joj je istekao period, i da moze ponovo da se izvrsava*/
void * task_jedan(void *id)
{
	while(sem_trywait(&kraj)!=0)
	{
		if(sem_trywait(&t1)==0)
		{
			task1();
		}
	}
	sem_post(&kraj);
	return NULL;
}

/*funkcija za nit koja predstavlja task 2, ceka da joj se signalizira da joj je istekao period, i da moze ponovo da se izvrsava*/
void * task_dva(void *id)
{
	while(sem_trywait(&kraj)!=0)
	{
		if(sem_trywait(&t2)==0)
		{
			task2();
		}
	}
	sem_post(&kraj);
	return NULL;
}
/*Funkcija za nit koja pokrece tajmer, koji okida na svakih 10ms, sa ofsetom 2s*/
void * timer_start(void* i)
{
	int res;
	res = start_periodic_timer(2000, 10);
    if (res < 0) {
        perror("Start Periodic Timer");

        return NULL;
    }

    while(sem_trywait(&kraj)!=0) //dok nije postavljen semafor za kraj
    {
        wait_next_activation();
    }
    sem_post(&kraj);

return NULL;
}
int main(int argc, char *argv[])
{
	pthread_t task1, task2,kontrolna;
	int kod_greske;
	const int signal = SIGALRM;

	sigset_t alarm_sig;
	sigemptyset(&alarm_sig);//ispraznimo sigset jer je inicjalno dobio nesto
    sigaddset(&alarm_sig, signal);//dodamo signale koje mi trebamo
    sigprocmask(SIG_BLOCK, &alarm_sig, NULL);
	
	printf("Kraj programa -> enter\n");
	
	if(sem_init(&kraj,0,0)) return printf("Semafor kraj nije keriran!");
	if(sem_init(&t1,0,0)) return printf("Semafor za task 1 nije keriran!");
	if(sem_init(&t2,0,0)) return printf("Semafor za task 2 nije keriran!");
	
	kod_greske=pthread_create(&(kontrolna),NULL,timer_start,NULL);
		if(kod_greske) return printf("Nit koja regulise program! Kod greske %d",kod_greske);
	kod_greske=pthread_create(&(task1),NULL,task_jedan,NULL);
		if(kod_greske) return printf("Nit za task 1 nije kerirana! Kod greske %d",kod_greske);
	kod_greske=pthread_create(&(task2),NULL,task_dva,NULL);
		if(kod_greske) return printf("Nit za task 2 nije kerirana! Kod greske %d",kod_greske);
		
	getchar();
	sem_post(&kraj);
	
	pthread_join(task1,NULL);
	pthread_join(task2,NULL);
	pthread_join(kontrolna,NULL);
	sem_destroy(&kraj);
	sem_destroy(&t1);
	sem_destroy(&t2);

    return 0;
}
