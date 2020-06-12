#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>


static sigset_t sigset;
static sem_t kraj;
void task1(void)
{
  int i,j;
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
}

void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
}

void task3(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<100000; j++) ;
    printf("3");
    fflush(stdout);
  }
}
/*ceka SIGALRM od tajmera*/
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

    t.it_value.tv_sec = offs / 1000;
    t.it_value.tv_nsec = (offs - (offs / 1000)*1000) * 1000*1000;
    t.it_interval.tv_sec = period / 1000;
    t.it_interval.tv_nsec = ( period - (period / 1000)*1000) *1000*1000;
	
    sigemptyset(&sigset);//ispraznimo sigset jer je inicjalno dobio nesto
    sigaddset(&sigset, signal);//dodamo signale koje mi trebamo
    sigprocmask(SIG_BLOCK, &sigset, NULL);
	
	// alociranje i popunjavanje sigevent strukture
    memset(&sigev, 0, sizeof(struct sigevent));
    sigev.sigev_notify = SIGEV_SIGNAL; 
    sigev.sigev_signo = signal;
	// kreiranje tajmera
    res = timer_create(CLOCK_MONOTONIC, &sigev, &timer);
    if (res < 0) {
        perror("Timer Create");

	exit(-1);
    }
	// pokretanje tajmera 
    return timer_settime(timer, 0 /*TIMER_ABSTIME*/, &t, NULL);
}

/*funkcija za obradjivanje signala za kraj*/
void sig_handler(int signo)
{
  if (signo == SIGRTMIN)
			sem_post(&kraj);
}

/*ispisuje kako je proces zavrsio, odnosno njegov status*/
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

/*setuje prioritet djetata i kreira tajmer sa odredjenim periodom i ofsetom za svako dijete i zove task funkciju koju treba za dato dijete*/
int child_code(int priority, int period, int offset, void (*f)(void))
{
		struct sched_param param; 
		param.sched_priority = priority; 
		sched_setscheduler(0, SCHED_FIFO, &param);
		    
		 /*pokretanje tajmera sa ofsetom od 2s i periodom 40ms*/   
		int res;
		res = start_periodic_timer(offset, period);
		if (res < 0) {
			perror("Start Periodic Timer");

			return -1;
		}
		/*hvatam signal za prekid taska i obradjujem ga u funkciji sig_handler*/
		signal(SIGRTMIN, sig_handler);
		while( sem_trywait(&kraj) != 0 ) 
		{
			wait_next_activation();
			f();
		}
		return period;
		// child code
}
int main(int argc, char *argv[])
{
    pid_t child1,child2,child3;
    int status1,status2,status3,priority;
    printf("Kraj programa -> enter\n");
    if(sem_init(&kraj,0,0)) return printf("Semafor kraj nije keriran!");
    child1 = fork();
    if (child1 < 0) 
    {
        perror("Fork 1");
        return -1;
    }
    if (child1 == 0) 
    {
		//printf("Child 1[%d]:\tParentID = %d\n", getpid(), getppid());
		priority = sched_get_priority_max(SCHED_FIFO);
		return child_code(priority,40,2000,&task1);
    }
    
	child2 = fork();
    if (child2 < 0)
    {
        perror("Fork 2");
        return -1;
    }
    if (child2 == 0) 
    {
		//printf("Child 2[%d]:\tParentID = %d\n", getpid(), getppid());
		priority = sched_get_priority_min(SCHED_FIFO);
		return child_code(priority+10,80,2000,&task2);
    }
    child3 = fork();
     if (child3 < 0) 
     {
        perror("Fork 3");
        return -1;
    }
    if (child3 == 0) 
    {
		//printf("Child 3[%d]:\tParentID = %d\n", getpid(), getppid());
		
		priority = sched_get_priority_min(SCHED_FIFO);
		return child_code(priority+1,120,2000,&task3);
    }
    
    // Parent code    

	priority = sched_get_priority_min(SCHED_FIFO);
	struct sched_param param; 
	param.sched_priority = priority; 
	sched_setscheduler(0, SCHED_FIFO, &param);
	
    getchar();
    /*SIGRTMIN je jedan od signala za rad u realnom vremenu*/
	if (kill (child1, SIGRTMIN) == -1) 
	{
		printf ("kill of child 1 failed"); 
		return -1;
	}		
	if (kill (child2, SIGRTMIN) == -1) 
	{
		printf ("kill of child 2 failed"); 
		return -1;
	}
	if (kill (child3, SIGRTMIN) == -1) 
	{
		printf ("kill of child 3 failed"); 
		return -1;
	}
	
    /*roditelj treba sacekati dijecu*/
    if (waitpid (child1, &status1, 0) == -1) 
    {
		perror ("waitpid failed"); 
		return -1;
	}    
	if (waitpid (child2, &status2, 0) == -1) 
    {
		perror ("waitpid failed"); 
		return -1;
	}
	    if (waitpid (child3, &status3, 0) == -1) 
    {
		perror ("waitpid failed"); 
		return -1;
	}
	
	/*ispisivanje kako su dijeca unistena*/
	printf("\n");
    print_child_status(status1,"1");
	print_child_status(status2,"2");
	print_child_status(status3,"3");


    return 0;
}
