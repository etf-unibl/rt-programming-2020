#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include "data.h"

/* Funkcija za postavljanje prioriteta programske niti! */   
static void setprio(int prio, int sched){
	struct sched_param param;

   	// podesavanje prioriteta i schedulera
   	param.sched_priority = prio;
   	if (sched_setscheduler(0, sched, &param) < 0)
   		perror("sched_setscheduler");
}

/* Ispis statusa child procesa!*/
void print_child_status (int status, int child_number) {
	if (WIFEXITED (status)) {
		printf("Child %d exited with status %d!\n", child_number, WEXITSTATUS (status));
	} else if (WIFSTOPPED (status)) {
		printf("Child %d stopped by signal %d (%d)!\n", child_number, WSTOPSIG (status),
	    strsignal(WSTOPSIG (status)));
	} else if (WIFSIGNALED (status)) {
		printf("Child %d killed by signal %d (%d)!\n", child_number, WTERMSIG (status),
	    strsignal(WTERMSIG (status)));
	} else {
		printf("Unknown child status!\n");
	}
}

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


int main(int argc, char *argv[])
{	
	int i,j;
	pid_t child[BROJ_TASKOVA];
	int status[BROJ_TASKOVA];
	
	/* Adekvatno popunjavanje struktura! */
	STRUKTURA_TASK struktura_task[BROJ_TASKOVA] = 
		{{.offs = OFFSET_0,.period = PERIOD_0, .task_job = task_job_1,.prioritet=sched_get_priority_max(SCHED_FIFO)},
		{.offs = OFFSET_1,.period = PERIOD_1, .task_job = task_job_2,.prioritet=sched_get_priority_max(SCHED_FIFO)-1},
		{.offs = OFFSET_2,.period = PERIOD_2, .task_job = task_job_3,.prioritet=sched_get_priority_max(SCHED_FIFO)-2}};

	setprio(sched_get_priority_min(SCHED_FIFO), SCHED_FIFO);  	// Postavljanje prioriteta parent procesa!
	
	/* Kreiranje child procesa! */
	for(i=0;i<BROJ_TASKOVA;i++){
		/* Proces 'i'! */
    	child[i] = fork();
    	if (child[i] < 0) {
       		perror("Fork proces!");
       		return -1;
    	}
		
		/* Child proces, dio koda! */
   		 if (child[i] == 0) {
    	    printf("Child[%d]:\tParentID = %d\n", getpid(), getppid());

			setprio(struktura_task[i].prioritet, SCHED_FIFO);  	// Postavljanje prioriteta child procesa!

   			create_periodic_timer(&struktura_task[i]);	// Kreiranje tajmera!
			int res;

			res = timer_settime(struktura_task[i].timer, 0 /*TIMER_ABSTIME*/, &struktura_task[i].t, NULL);
    		if (res < 0) {
       			perror("Cannot Create Periodic Timer!");
      			exit(-1);
    		}
			
			int broj_ponavljanja = 10;
    		while(broj_ponavljanja--) {  //while(1){ 
    		    wait_next_activation(&struktura_task[i].sigset);	    // Cekanje na timer alarm!
				struktura_task[i].task_job();					// Obavljanje odgovarajuceg posla po aktiviranju alarma!
    		}
   		   	return 33;
		}

	}

	for(j=0;j<BROJ_TASKOVA;j++){
		// Cekanje parent procesa, na child procese!
    	wait(&status[j]);
    
		//Ispis statusa child procesa!
		print_child_status(status[j],j+1);
	}


    return 0;
}
