#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

static struct timespec r[3];
static int PERIOD[3];

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

static void wait_next_activation(int i)
{
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &r[i], NULL);
    timespec_add_us(&r[i], PERIOD[i]);
}
int start_periodic_timer(uint64_t offs, int t, int i)
{
    clock_gettime(CLOCK_REALTIME, &r[i]);
    timespec_add_us(&r[i], offs);
    PERIOD[i] = t;
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

static void function(const int i)
{
    switch(i)
    {
        case 0:
            while(1)
            {
                task1();
                wait_next_activation(i);
            }
        case 1:
            while(1)
            {
                task2();
                wait_next_activation(i);
            }
        case 2:
            while(1)
            {
                task3();
                wait_next_activation(i);
            }
    }
}

int main()
{
    int res;
    res = start_periodic_timer(0, 40000,0);
        if (res < 0) {
                perror("Start Periodic Timer");
                return -1;
            }
    res = start_periodic_timer(0, 80000,1);
        if (res < 0) {
                perror("Start Periodic Timer");
                return -1;
            }
    res = start_periodic_timer(0, 120000,2);
        if (res < 0) {
                perror("Start Periodic Timer");
                return -1;
            }
    int i;
    for(i=0; i<3; i++)
    {
        int child=fork();
        if(child < 0)
        {
             perror("Fork");
             return -1;
        }
        else if(child>0)
        {
            //parrent process
        }
        else
        {
		    int pmin = sched_get_priority_min(SCHED_FIFO);
		    struct sched_param param; 
		    param.sched_priority = pmin + (3-i); 
		    sched_setscheduler(0, SCHED_FIFO, &param);

            printf("Child[%d]:\tParentID = %d\n", getpid(), getppid());
            function(i);
            exit(EXIT_SUCCESS);
        }
    }
    for (i = 0; i < 3; i++) {
        int status;
        pid_t pid = wait(&status);  //any order
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
    return 0;
}
