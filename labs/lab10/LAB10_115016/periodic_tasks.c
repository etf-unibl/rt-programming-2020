#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#define BROJ_TASKOVA 2

/* Enumeracija vremena aktivacije! */
enum VREMENA {   OFFSET_0 = 1000000,
				 OFFSET_1 = 600000,	    //600ms!
				 PERIOD_0 = 2000000,
				 PERIOD_1 = 800000		//800ms!
				};


/* Struktura vezana za task!*/
typedef struct{
	struct itimerspec t;
    timer_t timer;
	sigset_t sigset;
	uint64_t offs;
	uint32_t period;
	void (*task_job)();	// Pokazivac na funkciju JOB programske niti! 
}STRUKTURA_TASK;


/* Koriscenje sigwaita, za cekanje alarma! */
static void wait_next_activation(sigset_t* sigset)
{
    int dummy;
    sigwait(sigset, &dummy);
}

/* Kreiranje periodicnog tajmera! */
void create_periodic_timer(STRUKTURA_TASK *struktura_task)
{
    struct sigevent sigev;
    const int signal = SIGALRM;
    int res;

    struktura_task->t.it_value.tv_sec = struktura_task->offs / 1000000;
    struktura_task->t.it_value.tv_nsec = (struktura_task->offs % 1000000) * 1000;
    struktura_task->t.it_interval.tv_sec = struktura_task->period / 1000000;
    struktura_task->t.it_interval.tv_nsec = (struktura_task->period % 1000000) * 1000;
	
	// Ciscenje, te popunjavanje sigset_t strukture!
    sigemptyset(&struktura_task->sigset);
    sigaddset(&struktura_task->sigset, signal);
    sigprocmask(SIG_BLOCK, &struktura_task->sigset, NULL);

	// alociranje i popunjavanje sigevent strukture
    memset(&sigev, 0, sizeof(struct sigevent));
    sigev.sigev_notify = SIGEV_SIGNAL; // Kada vrijeme istekne, okinuce se signal koji smo prethodno definisali kao SIGALRM!					   
    sigev.sigev_signo = signal;	
	
	// Kreiranje tajmera!
    res = timer_create(CLOCK_MONOTONIC, &sigev, &struktura_task->timer);
    if (res < 0) {
        perror("Timer Create");

	exit(-1);
    }
}

/* Task job prvog taska! */
void task_job_1(){
	int i,j;
  		for (i=0; i<4; i++) {
			for (j=0; j<1000; j++) ;
    		printf("1");
    		fflush(stdout);
  		}
		printf("\n");
}

/* Task job drugog taska! */
void task_job_2(){
	int i,j;
  		for (i=0; i<6; i++) {
   			for (j=0; j<10000; j++) ;
    		printf("2");
    		fflush(stdout);
  		}
		printf("\n");
}

/* Funkcija taskova! */
void* periodicniTask(void *arg){
	STRUKTURA_TASK struktura_task = *(STRUKTURA_TASK*)arg;

	int res;

	res = timer_settime(struktura_task.timer, 0 /*TIMER_ABSTIME*/, &struktura_task.t, NULL);
    if (res < 0) {
       	perror("Cannot Create Periodic Timer!");
      	exit(-1);
    }

    while(1) {
        wait_next_activation(&struktura_task.sigset);	    // Cekanje na timer alarm!
		struktura_task.task_job();							// Obavljanje odgovarajuceg posla po aktiviranju alarma!
    }
 
}

/* Kreiranje programskih niti! */
static pthread_t start_rt_thread(STRUKTURA_TASK* struktura_task){
	pthread_t thread;

	// offset - uvodno kasnjenje, koliko treba da prodje do prvog aktiviranja!
	// period - period aktiviranja tajmera!
	// Izrazeno u sekundama pomnozenim sa 1_000_000!

   	create_periodic_timer(struktura_task);	// Kreiranje tajmera!
  	/* Kreiranje programske niti */
   	pthread_create(&thread, NULL, periodicniTask , (void*)struktura_task);

   	return thread;
} 


int main(int argc, char *argv[])
{
  	pthread_t task[BROJ_TASKOVA];

	/* Adekvatno popunjavanje strukture! */
	STRUKTURA_TASK struktura_task1 = {.offs = OFFSET_0,.period = PERIOD_0, .task_job = task_job_1};
	STRUKTURA_TASK struktura_task2 = {.offs = OFFSET_1,.period = PERIOD_1, .task_job = task_job_2};

	/* Kreiranje, i startovanje programskih niti! */
	task[0] = start_rt_thread(&struktura_task1);
	task[1] = start_rt_thread(&struktura_task2);

	// Join nad prethodno kreiranim nitima!
	pthread_join(task[0],NULL);
	pthread_join(task[1],NULL);
    return 0;
}

