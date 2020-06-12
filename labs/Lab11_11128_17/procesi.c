#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>

#define NSEC_PER_SEC 1000000000ULL
static sem_t sem_end;

static inline void timespec_add_us(struct timespec *t, uint64_t d);
static void wait_next_activation(struct timespec* r, int period);
int start_periodic_timer( struct timespec* r, uint64_t offs);
void task1(void);
void task2(void);
void task3(void);
static void job_body(int period_timer);
void signal_handler(int signo);

int main(int argc, char *argv[])
{
    int res;
	struct timespec r;
	struct sigaction sig;
	int period_timer;
	int offset_timer;
	
	/* Popunjavanje strukture 'sigaction' sa funkcijom koja se pozoviva po prijemu signala SIGRTMIN. */
	sig.sa_handler = &signal_handler;
	
	/* Provjera argumenata. */
	if(argc != 3)
	{	
		printf("Greska, provjerite argumente!\nArgumenti treba da budu period u 'ms' i offset u 'ms'!\n");
		return -1;
	}
	/* Formiranje semafora. */
	if ( sem_init(&sem_end, 0, 0) != 0 )
	{
		printf("Greska, semafor ""sem_end"" nije formiran!\n");
		return -1;
	}
	
	period_timer = atoi(argv[1]);
	offset_timer = atoi(argv[2]);
	
	/* Pokretanje tajmera. */
    res = start_periodic_timer(&r,offset_timer);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }
	
	/* Pokretanje funkcije signal_handler po prijemu signala SIGRTMIN. */
	sigaction(SIGRTMIN,&sig,NULL);

	/* Ponavlja se sve dok se ne postavi semafor za kraj programa. */
    while( sem_trywait(&sem_end) != 0 ) {
        wait_next_activation(&r,period_timer);
        job_body(period_timer);
    }
	
	/* Unistavanje semafora. */
	sem_destroy(&sem_end);
		
    return 24;
}

/* Funkcija koja racuna sledeci istek roka. */
static inline void timespec_add_us(struct timespec *t, uint64_t d)
{
    d *= 1000000;
    d += t->tv_nsec;
    while (d >= NSEC_PER_SEC) {
        d -= NSEC_PER_SEC;
		t->tv_sec += 1;
    }
    t->tv_nsec = d;
}
/* Funkcija koja ceka do sledece aktivacije funkcije job_body. */
static void wait_next_activation(struct timespec* r, int period)
{
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, r, NULL);
    timespec_add_us(r, period);
}

int start_periodic_timer( struct timespec* r, uint64_t offs)
{
    clock_gettime(CLOCK_REALTIME, r);
    timespec_add_us(r, offs);

    return 0;
}

/* Periodicni zadatak na 40 milisekundi. */
void task1(void)
{
  int i,j;
 
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
}

/* Periodicni zadatak na 80 milisekundi. */
void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
}

/* Periodicni zadatak na 120 milisekundi. */
void task3(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<100000; j++) ;
    printf("3");
    fflush(stdout);
  }
}

/* Funkcija koja pokrece odgovarajuci task nakon perioda. */
static void job_body(int period_timer)
{
    switch(period_timer){
		case 40 : 
			task1();
			break;
		case 80 : 
			task2();
			break;
		case 120 : 
			task3();
			break;
		default: 
			sem_post(&sem_end);
	}
}

/* Funkcija koja se pogrece kada se primi signal SIGRTMIN, postavlja semafor za kraj programa. */
void signal_handler(int signo)
{
	if(signo == SIGRTMIN ){
		sem_post(&sem_end);
	}
}
