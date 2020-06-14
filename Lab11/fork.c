#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

static struct timespec r;
static int period;

#define NSEC_PER_SEC 1000000000ULL

static inline void timespec_add_us(struct timespec *t, uint64_t d)
{
    d *= 1000;
    d += t->tv_nsec;
    while (d >= NSEC_PER_SEC) {
        d -= NSEC_PER_SEC;
		t->tv_sec += 1;
    }
    t->tv_nsec = d;
}

static void wait_next_activation(void)
{
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &r, NULL); 
    timespec_add_us(&r, period); 
}

int start_periodic_timer(uint64_t offs, int t)
{
    clock_gettime(CLOCK_REALTIME, &r); 
    timespec_add_us(&r, offs);  
    period = t;  
    return 0;
}

// periodicni zadatak na 40 milisekundi
void task1(void)
{
  int i,j;
 
  	for (i=0; i<4; i++) {
    	for (j=0; j<1000; j++) ;
    	printf("1");
    	fflush(stdout);
  }
}

// periodicni zadatak na 80 milisekundi
void task2(void)
{
  int i,j;

  	for (i=0; i<6; i++) {
    	for (j=0; j<10000; j++) ;
    	printf("2");
   	 	fflush(stdout);
  }
}

// periodicni zadatak na 120 milisekundi
void task3(void)
{
  int i,j;

  	for (i=0; i<6; i++) {
    	for (j=0; j<100000; j++) ;
    	printf("3");
    	fflush(stdout);
  }
}

void status_print(int status)
{
	wait(&status);
    
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
}


int main(int argc, char *argv[])
{
	pid_t periodic[3];
	int status, res;
	
	periodic[0] = fork();
	if (periodic[0] < 0) {
        perror("Fork");
        return -1;
    }
	
	if(periodic[0]==0)
	{
		res = start_periodic_timer(2000000, 40000);
        if (res < 0) 
        {
                perror("Start Periodic Timer");
                return -1;                
        }
		
		int pmin = sched_get_priority_min(SCHED_FIFO);
		struct sched_param param; 
		param.sched_priority = pmin + 120; 
		sched_setscheduler(0, SCHED_FIFO, &param);
		printf("CurentTaskID[%d]:\tParentID = %d\n", getpid(), getppid());
		while(1)
		{
			task1();
			wait_next_activation();
		}
	}
        
	periodic[1] = fork();
	if (periodic[1] < 0) {
        perror("Fork");
        return -1;
    }
	
	if(periodic[1]==0)
	{
		res = start_periodic_timer(2500000, 80000);
        if (res < 0) 
        {
                perror("Start Periodic Timer");
                return -1;                
        }
		int pmin = sched_get_priority_min(SCHED_FIFO);
		struct sched_param param; 
		param.sched_priority = pmin + 80; 
		sched_setscheduler(0, SCHED_FIFO, &param);
		printf("Child[%d]:\tParentID = %d\n", getpid(), getppid());
		while(1)
		{
			task2();
			wait_next_activation();
		}
	}
    
	periodic[2] = fork();
	if (periodic[2] < 0) {
        perror("Fork");
        return -1;
    }
	
	if(periodic[2]==0)
	{
		res = start_periodic_timer(3000000, 120000);
        if (res < 0) 
        {
                perror("Start Periodic Timer");
                return -1;                
        }
		int pmin = sched_get_priority_min(SCHED_FIFO);
		struct sched_param param; 
		param.sched_priority = pmin + 40; 
		sched_setscheduler(0, SCHED_FIFO, &param);
		printf("Child[%d]:\tParentID = %d\n", getpid(), getppid());
		while(1)
		{
			task3();
			wait_next_activation();
		}
	}
	
	for(int i=0;i<3;++i)
		status_print(status);
	
    return 0;
}
