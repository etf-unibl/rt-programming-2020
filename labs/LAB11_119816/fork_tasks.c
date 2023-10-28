#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>

static sigset_t sigset;
sigset_t alarm_sig;

void task1(void)
{
  int i,j;
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("O");
    //printf("1");
    fflush(stdout);
  }
}

void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    printf("|");
    //printf("2");
    fflush(stdout);
  }
}

void task3(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<100000; j++) ;
    printf("#");
    //printf("3");
    fflush(stdout);
  }
}

void print_child_status (int status,char* id) {
	if (WIFEXITED (status)) {
		printf("Child %s exited with status %d\n", id,WEXITSTATUS (status));
	} else if (WIFSTOPPED (status)) {
		printf("Child %s stopped by signal %d (%d)\n", id,WSTOPSIG (status),
	    atoi(strsignal(WSTOPSIG (status))));
	} else if (WIFSIGNALED (status)) {
		printf("Child %s killed by signal %d (%d)\n", id,WTERMSIG (status),
	    atoi(strsignal(WTERMSIG (status))));
	} else {
		printf("Unknown child status\n");
	}
}

static void wait_period(void)
{
	int sig;
	/* Wait for the next SIGALRM */
	sigwait(&alarm_sig, &sig);
}


static int make_periodic(unsigned int period)
{
	int error;
	struct itimerval value;
	/* Block SIGALRM in this thread */
	sigemptyset(&alarm_sig);
	sigaddset(&alarm_sig, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &alarm_sig, NULL);
	/* Set the timer to go off after the first period and then
	   repetitively */
	value.it_value.tv_sec = period / 1000000;
	value.it_value.tv_usec = period % 1000000;
	value.it_interval.tv_sec = period / 1000000;
	value.it_interval.tv_usec = period % 1000000;
	if ((setitimer(ITIMER_REAL, &value, NULL))!= 0)
		printf("Error creating timer %d.\n",error);
	return error;
}

int child_code(int priority, int period, void (*func_pointer)(void))
{
		struct sched_param shared_param; 
		shared_param.sched_priority = priority; 
		sched_setscheduler(0, SCHED_FIFO, &shared_param);
		    
		if ((make_periodic(period)) < 0){
			perror("Start Periodic Timer Error");
			return -1;
		}
	    while(1){
			wait_period();
			func_pointer();
		}
		return period;
}
int main(int argc, char *argv[])
{
    int Timer;
    pid_t child_1;
    pid_t child_2;
    pid_t child_3;
    int priority;
    int child_stat_1,child_stat_2,child_stat_3;
   printf("1= O ; 2= | ; 3= # ; --this is for a better visual representation\n ");
   printf("Enter the running time for the program (seconds):\n");
   scanf("%d",&Timer);
    if ((child_1=fork()) < 0){
        printf("Fork for child_1 error %d.\n",child_1);
        exit(child_1);
    }
    if (child_1 == 0){
		priority = sched_get_priority_max(SCHED_FIFO);
		return child_code(priority,40,&task1);
    }
    if ((child_2 = fork()) < 0){
        printf("Fork for child_2 error %d.\n",child_2);
        exit(child_2);
    }
    if (child_2 == 0){
		priority = sched_get_priority_min(SCHED_FIFO);
		return child_code(priority+2,80,&task2);
    }
    
    if ((child_3 = fork()) < 0){
        printf("Fork for child_3 error %d.\n",child_3);
        exit(child_3);
    }
    if (child_3 == 0){
		priority = sched_get_priority_min(SCHED_FIFO);
		return child_code(priority+3,120,&task3);
    }
    // Parent code ---------------------------------   
	priority = sched_get_priority_min(SCHED_FIFO);
	struct sched_param shared_param; 
	shared_param.sched_priority = priority; 
	sched_setscheduler(0, SCHED_FIFO, &shared_param);
    // Sleep main for some time to let the child proc execute
	sleep(Timer);
	if (kill (child_1, SIGRTMIN) == -1){
		printf ("Error killing child_1\n"); 
		exit(-1);
	}		
	if (kill (child_2, SIGRTMIN) == -1){
	    printf ("Error killing child_2\n"); 
		exit(-1);
	}
	if (kill (child_3, SIGRTMIN) == -1){
	    printf ("Error killing child_3\n"); 
		exit(-1);
	}
    if (waitpid (child_1, &child_stat_1, 0) == -1){
		printf("Waiting for child_1 failed\n");
		exit(-1);
	}    
	if (waitpid (child_2, &child_stat_2, 0) == -1){
		printf("Waiting for child_1 failed\n");
		exit(-1);
	}
	if (waitpid (child_3, &child_stat_3, 0) == -1){
		printf("Waiting for child_1 failed\n");
		exit(-1);
	}
	printf("\n");
	print_child_status(child_stat_1,"1");
	print_child_status(child_stat_2,"2");
	print_child_status(child_stat_3,"3");

    return 0;
}


