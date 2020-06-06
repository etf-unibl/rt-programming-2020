#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

//staticke promjenljive
static sigset_t sigset_task;	//set signala
static pthread_t t1, t2, st;	//tredovi
static sem_t s1, s2;			//semafori
static int kraj = 0;			//promjenljiva koja se setuje na 1 kada se zavrsi program

//Funckija koja namjesta tajmer
int start_periodic_timer(uint64_t offs, int period)
{
    struct itimerval t;

    t.it_value.tv_sec = offs / 1000000;
    t.it_value.tv_usec = offs % 1000000;
    t.it_interval.tv_sec = period / 1000000;
    t.it_interval.tv_usec = period % 1000000;
	
	sigemptyset(&sigset_task);
	sigaddset(&sigset_task, SIGALRM);
	sigprocmask(SIG_BLOCK, &sigset_task, NULL);
	
	return setitimer(ITIMER_REAL, &t, NULL);
}
//Funkcija koja prestavlja thread za Task 1, svakih 60us ispise '1', ceka na semafor s1
void *task1(void* pParam){
	int i, j;
	for (i=0; i<4; i++) {
		sem_wait(&s1);
		for (j=0; j<1000; j++) ;
		printf("1");
		fflush(stdout);
	}
	return 0;
}
//Funkcija koja prestavlja thread za Task 2, svakih 80us ispise '2', ceka na semafor s2 pa spava jos 20u
void *task2(void* pParam){
	int i, j;
	for (i=0; i<6; i++) {
		sem_wait(&s2);
		usleep(20);
		for (j=0; j<10000; j++) ;
		printf("2");
		fflush(stdout);
	}
	return 0;
}
//Funkcija koja ceka na signal od tajmera koji je namjesten na 60us pa signalizira semafore s1 i s2, radi sve dok je kraj == 0
void *semaphore_thread(void*pParam){
	int dummy;
	while(kraj == 0){
		sigwait(&sigset_task, &dummy);
		sem_post(&s1);
		sem_post(&s2);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	//kreiranje tajmera
	int res;
    res = start_periodic_timer(2000000, 60000);
    if (res < 0) {
        perror("Start Periodic Timer");
        return -1;
    }
    //inicjalizovanje semafora s1 i s2
    sem_init(&s1,0,0);
    sem_init(&s2,0,0);
    //kreiranje thredova
    pthread_create(&st,NULL,semaphore_thread,NULL);
    pthread_create(&t1,NULL,task1,NULL);
    pthread_create(&t2,NULL,task2,NULL);
    //cekanje na kraj izvrsavanja taskova 1 i 2
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    //oznacavanje kraja programa, da se zaustavi thread za pracenje tajmera
    kraj = 1;
    //unistavanje semafora s1 i s2
    sem_destroy(&s1);
    sem_destroy(&s2);

    return 0;
}
