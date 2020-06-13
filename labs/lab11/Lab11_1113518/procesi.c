#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <stdio.h>
#include <sched.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>

//staticke promjenljive
static sigset_t sigset_task;	//set signala

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

void wait_next_activation(void)
{
	int dummy;
	// ceka signal iz seta i u dummy upisuje
	// signal koji je dosao
	sigwait(&sigset_task, &dummy); 
	// ukoliko imamo vise signala onda bi ovdje
	// trebalo ispitati dummy
	// if (dummy == SIGALRM){}
}

// periodicni zadatak na 40 milisekundi
void task1(void){
	int i,j;
	for (i=0; i<4; i++) {
		wait_next_activation();
		for (j=0; j<1000; j++);
		printf("1");
		fflush(stdout);
	}
}

// periodicni zadatak na 80 milisekundi
void task2(void){
	int i,j;
	for (i=0; i<6; i++) {
		wait_next_activation();
		for (j=0; j<10000; j++);
		printf("2");
		fflush(stdout);
	}
}

// periodicni zadatak na 120 milisekundi
void task3(void){
	int i,j;
	for (i=0; i<6; i++) {
		wait_next_activation();
		for (j=0; j<100000; j++);
		printf("3");
		fflush(stdout);
	}
}

int main(int argc, char*argv[]){
	pid_t child1, child2, child3;
    int status;
    
    child1 = fork();
    if (child1 < 0) {
        perror("Fork");
        return -1;
    }
    if (child1 == 0) {
		// child1 code
		struct sched_param param; 
		param.sched_priority = sched_get_priority_max(SCHED_FIFO); 
		sched_setscheduler(0, SCHED_FIFO, &param);
		
		//kreiranje tajmera
		int res;
		res = start_periodic_timer(2000000, 40000);
		if (res < 0) {
			perror("Start Periodic Timer");
			return -1;
		}
		
		task1();
		
		return 11;
		// child1 code
    }
    
    child2 = fork();
    if (child2 < 0) {
        perror("Fork");
        return -1;
    }
    if (child2 == 0) {
		// child2 code
		struct sched_param param; 
		param.sched_priority = (sched_get_priority_max(SCHED_FIFO) - sched_get_priority_min(SCHED_FIFO)) / 2; 
		sched_setscheduler(0, SCHED_FIFO, &param);
		
		//kreiranje tajmera
		int res;
		res = start_periodic_timer(2000000, 80000);
		if (res < 0) {
			perror("Start Periodic Timer");
			return -1;
		}
		
		task2();
		
		return 22;
		// child2 code
    }
    
	struct sched_param param; 
	param.sched_priority = sched_get_priority_min(SCHED_FIFO); 
	sched_setscheduler(0, SCHED_FIFO, &param);
	int res;
	res = start_periodic_timer(2000000, 120000);
	if (res < 0) {
		perror("Start Periodic Timer");
		return -1;
	}
		
	task3();
	
	wait(&status);
	wait(&status);
	/*
	//print waited child status
	if (WIFEXITED (status)) {
		printf("Child exited with status %d\n", WEXITSTATUS (status));
	} else if (WIFSTOPPED (status)) {
		printf("Child stopped by signal %d (%d)\n", WSTOPSIG (status),
	    strsignal(WSTOPSIG (status)));
	} else if (WIFSIGNALED (status)) {
		printf("Child killed by signal %d (%d)\n", WTERMSIG (status),
	    strsignal(WTERMSIG (status)));
	} else {
		printf("Unknown child status\n");
	}
	*/
	return 0;
}
