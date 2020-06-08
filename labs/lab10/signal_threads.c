#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include <semaphore.h>

/*
 * Iz dokumentacije za setitimer:
 * 	The system provides each process with three interval timers, each decrementing in a distinct time domain.
 * 	When any timer expires, a signal is sent to the process, and the timer (potentially) restarts.
 * 
 * Iz dokumentacije za sigwait, sigwaitinfo i sigtimedwait:
 * 	If multiple threads of a process are blocked waiting for the same signal(s) in sigwaitinfo() or sigtimedwait(), then exactly one of 
 * 	the threads will actually receive the signal if it becomes pending for the process as a whole; which of the threads receives the 
 * 	signal is indeterminate.
 *  
 * Ako bi obe niti čekale na SIGALRM tada bi došlo do trke izmedju njih za signal. Samo jedna bi dobila signal i nije garantovano da
 * bi signal generisan tajmerom za prvu nit pokupila prva nit. Može se desiti da ga pokupi druga nit i da ona odradi svoj zadatak u periodu
 * kada bi to trebala prva.
 * Da bi se izbjeglo ovo inicijalizovaće se jedan tajmer koji će slati SIGALRM svakih 20us. Nit koja izvršava main funkciju će
 * primati te signale i pomoću semafora obavještavati niti koje imaju periodične zadatke kada je proteklo dovoljno vremena.
 * */

static int control=0;

typedef struct in{
	int x;
	int y;
	int info;
	sem_t* semaphore;
}INFO;

void* listen(void* arg){
    fgetc(stdin);
    control=1;
	return NULL;
}

//Periodični zadaci koje je trebalo realizovati se dobijaju proslijedjivanjem parametara funkciji periodic_task.
//Po logici videa petlja je ugnježdena, ali u kodu u dostavljenom fajlu nije. Ovdje je realizovana kako ugnježdena.
void* periodic_task(void* arg){
	INFO* a=(INFO*)arg;
	int i,j;
	while(1){
		sem_wait(a->semaphore);
		for (i=0; i<a->x; i++) {
			for (j=0; j<a->y; j++){
				printf("%d",a->info);
				fflush(stdout);
			}
		}
	}
	return NULL;
}

int main(){
	int i=0,j=0, dummy;
	
	sem_t task1, task2;
	sem_init(&task1,0,0);
	sem_init(&task2,0,0);
	
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGALRM);
	sigprocmask(SIG_BLOCK,&set,NULL);
	
	struct itimerval t;
    t.it_value.tv_sec=0;
    t.it_value.tv_usec=20000;
    t.it_interval.tv_sec=0;
    t.it_interval.tv_usec=20000;
	
	pthread_t t1, t2, l;
	INFO i1,i2;
	i1.x=4;
	i1.y=1000;
	i1.info=1;
	i1.semaphore=&task1;
	i2.x=6;
	i2.y=10000;
	i2.info=2;
	i2.semaphore=&task2;
	control=control+pthread_create(&t1,NULL,periodic_task,(void*)(&i1));
	if(control==0){
		control=control+pthread_create(&t2,NULL,periodic_task,(void*)(&i2));
		if(control==0){
			control=control+pthread_create(&l,NULL,listen,NULL);
			if(control==0){
				if(setitimer(ITIMER_REAL,&t,NULL)==0){
					while(control==0){
						sigwait(&set,&dummy);
						//dummy se ne provjerava jer je jedini signal u setu SIGALRM
						i=(i+1)%3;
						if(i==0){
							sem_post(&task1);
						}
						j=(j+1)%4;
						if(j==0){
							sem_post(&task2);
						}
					}
				}
				else{
					printf("Tajmer nije inicijalizovan!\n");
				}
			}
			else{
				printf("Neuspjesna inicijalizacija niti!\n");
			}
			pthread_cancel(t2);
			pthread_join(t2,NULL);
		}
		else{
			printf("Neuspjesna inicijalizacija niti!\n");
		}
		pthread_cancel(t1);
		pthread_join(t1,NULL);
	}
	else{
		printf("Neuspjesna inicijalizacija niti!\n");
	}
	
	sem_destroy(&task1);
	sem_destroy(&task2);
	
	printf("\n");
	
	return 0;
}
