#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>


#define NSEC_PER_SEC 1000000000ULL

/*Parametri za task-ove*/
typedef struct{
    struct timespec r;
    int period;
} task_param;


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


static void wait_next_activation(struct timespec *r, int period)
{
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, r, NULL);
    
    /*Setuje period*/
    timespec_add_us(r, period);
}


int start_periodic_timer(uint64_t offs, int t, struct timespec *r, int *period)
{
    /*Uzimamo realno vrijeme*/
    clock_gettime(CLOCK_REALTIME, r);
    /*Setuje offset*/
    timespec_add_us(r, offs);

    *period = t;

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


int main(int argc, char *argv[])
{
    int res;

    /*Parametri za task-ove*/
    task_param param1;
    task_param param2;
    task_param param3;

    pid_t child1;
    pid_t child2;
    pid_t child3;

    int status;
    

    child1 = fork();
    if (child1 < 0) {
        perror("Fork1");
        return -1;
    }

    if (child1 == 0) {       
		// child1 code
        
        /*U ovoj funkciji setuje parametre za prvu nit*/
        res = start_periodic_timer(200000, 40000, &param1.r, &param1.period);
        if (res < 0) {
            perror("Start Periodic Timer1");
            return -1;
        }

        /*Setovanje prioriteta i nacina rasporedjivanja*/
		int priority_min = sched_get_priority_min(SCHED_FIFO);
		struct sched_param param; 
		param.sched_priority = priority_min + 90; 
		sched_setscheduler(0, SCHED_FIFO, &param);                      
                                                                         
        printf("\nChild[%d]:\tParentID = %d\n", getpid(), getppid());
        while(1)
        {
            task1();
            wait_next_activation(&param1.r, param1.period);  
        }
		return 33;  
		// child1 code
    }

    child2 = fork();
    if (child2 < 0) {
        perror("Fork2");
        return -1;
    }
    
    if (child2 == 0) {
		// child2 code

        /*U ovoj funkciji setuje parametre za drugu nit*/
        res = start_periodic_timer(250000, 80000, &param2.r, &param2.period);
        if (res < 0) {
            perror("Start Periodic Timer2");
            return -1;
        }

        /*Setovanje prioriteta i nacina rasporedjivanja*/
		int priority_min = sched_get_priority_min(SCHED_FIFO);
		struct sched_param param; 
		param.sched_priority = priority_min + 60; 
		sched_setscheduler(0, SCHED_FIFO, &param);                    
                                                                    
        printf("\nChild[%d]:\tParentID = %d\n", getpid(), getppid());
        while(1)
        {
            task2();
            wait_next_activation(&param2.r, param2.period);  
        }
		return 33;  
		// child2 code
    }

    child3 = fork();
    if (child3 < 0) {
        perror("Fork3");
        return -1;
    }

    if (child3 == 0) {
		// child3 code

        /*U ovoj funkciji setuje parametre za trecu nit*/
        res = start_periodic_timer(300000, 120000, &param3.r, &param3.period);
        if (res < 0) {
            perror("Start Periodic Timer3");
            return -1;
        }

        /*Setovanje prioriteta i nacina rasporedjivanja*/
		int priority_min = sched_get_priority_min(SCHED_FIFO);
		struct sched_param param; 
		param.sched_priority = priority_min + 30; 
		sched_setscheduler(0, SCHED_FIFO, &param);                 
                                                                      
        printf("\nChild[%d]:\tParentID = %d\n", getpid(), getppid());
        while(1)
        {
            task3();
            wait_next_activation(&param3.r, param3.period);  
        }
		return 33; 
		// child3 code
    }
    

    /*Za svaki od child-ova po jedan wait*/
    wait(&status);
    wait(&status);
    wait(&status);

    printf("Parent[%d]:\tChild-ovi su u ovom trenutku sigurno zavrsili!\n",
		getpid());
    
    getchar();
    
    return 0;
}
