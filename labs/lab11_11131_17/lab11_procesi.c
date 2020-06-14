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

int offset=100;
int period=40000;   
void task1(void)
{
  int i,j;
 
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ; // 1 000
    printf("1");
    fflush(stdout);
  }
}

void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;// 10 000
    printf("2");
    fflush(stdout);
  }
}

void task3(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<100000; j++) ;// 100 000
    printf("3");
    fflush(stdout);
  }
}
void print_proces_status (int status,int id) {
	if (WIFEXITED (status)) {
		printf("Proces %d exited with status %d\n",id, WEXITSTATUS (status));
	} else if (WIFSTOPPED (status)) {
		printf("Proces %d stopped by signal %d (%s)\n",id, WSTOPSIG (status),
	    strsignal(WSTOPSIG (status)));
	} else if (WIFSIGNALED (status)) {
		printf("Proces %d killed by signal %d (%s)\n",id, WTERMSIG (status),
	    strsignal(WTERMSIG (status)));
	} else {
		printf("Unknown Proces status\n");
	}
}

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
    return timer_settime(timer, 0 /*TIMER_ABSTIME*/, &t, NULL);
}

void set_timer_and_priority(int t_period,int t_priority)
{
    printf("Child_proces__[%d]:\tParentID = %d\n", getpid(), getppid());

    struct sched_param param; 
	param.sched_priority = t_priority; 
    sched_setscheduler(0, SCHED_FIFO, &param);

    int res;
	res = start_periodic_timer(offset,t_period*period);
	if (res < 0) {
		perror("Start Periodic Timer 40 ms ");
		return -1;
	}
}

int main(int argc, char *argv[])
{
 int status1,status2,status3;
    pid_t proces1;
    pid_t proces2;
    pid_t proces3;
printf(" Pres ENTER for end!\n");
    proces1=fork();
if (proces1 < 0) 
    {
        perror("Fork ");
        return -1;
    }
if(proces1==0)
{
    set_timer_and_priority(1,99);// 1*40ms i prioritet 99
	while( 1 ) 
	{
		wait_next_activation();
		task1();
	};
}
    proces2=fork();
if (proces2 < 0) 
    {
        perror("Fork ");  
        return -1;
    }
if(proces2==0)
{
set_timer_and_priority(2,50);// 2*40ms period i prioritet 50
	while(1) 
	{
		wait_next_activation();
		task2();
	}                    
}
    proces3=fork();
if (proces3 < 0) 
    {
        perror("Fork ");
        return -1;
    }
if(proces3==0)
{
    set_timer_and_priority(3,0);// 3*40ms i prioritet 1
	while(1) 
	{
		wait_next_activation();
		task3();
	}
}
    // Parent code    

    getchar();       // roditelj ceka djecu dok ne udarimo Enter, poslije ih unisti i ispise status
 
if (kill (proces1, 9) == -1) 
	{
		printf ("kill of child 1 failed"); 
		return -1;
	}		
	if (kill (proces2, 9) == -1) 
	{
		printf ("kill of child 2 failed"); 
		return -1;
	}
	if (kill (proces3, 9) == -1) 
	{
		printf ("kill of child 3 failed"); 
		return -1;
}
    
 if (waitpid (proces1, &status1, 0) == -1) 
    {
		perror ("waitpid failed"); 
		return -1;
	}    
	if (waitpid (proces2, &status2, 0) == -1) 
    {
		perror ("waitpid failed"); 
		return -1;
	}
	    if (waitpid (proces3, &status3, 0) == -1) 
    {
		perror ("waitpid failed"); 
		return -1;
	}	
	printf("\n");
    print_proces_status(status1,1);
	print_proces_status(status2,2);
	print_proces_status(status3,3);

    return 0;
}
