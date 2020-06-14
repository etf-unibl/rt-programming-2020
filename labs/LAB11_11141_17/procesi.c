#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wait.h>
#include <signal.h>
#include <stdint.h>
#include <sched.h>
#include <sys/time.h>
#include <time.h>



typedef struct {
 
  uint64_t offs;
  int period;
}intervals;


intervals inter1={.offs=1000000,.period=40000};
intervals inter2={.offs=1000000,.period=80000};
intervals inter3={.offs=1000000,.period=120000};








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
   return timer_settime(timer, 0 /*TIMER_ABSTIME*/, &t, NULL);//izbacujemo
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



int proces1(intervals int1){

    //inicijalizacija
   // intervals int1=*(intervals*)arg;
    int res;
    //startovanje tajmera
    res = start_periodic_timer(int1.offs,int1.period);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }
 
   int pmax = sched_get_priority_max(SCHED_FIFO);
   struct sched_param param; 
   param.sched_priority = pmax-int1.offs; 
   sched_setscheduler(0, SCHED_FIFO, &param);
    
    while(1){

    //job_body
    task1();  
    //cekanje na sledecu aktivaciju 
    wait_next_activation();

    
  

   }
}




int proces2(intervals int2){

    //inicijalizacija
    //intervals int1=*(intervals*)arg;
    int res;
    //startovanje tajmera
    res = start_periodic_timer(int2.offs,int2.period);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }

   int pmax = sched_get_priority_max(SCHED_FIFO);
   struct sched_param param; 
   param.sched_priority = pmax-int2.offs; 
   sched_setscheduler(0, SCHED_FIFO, &param);

    
    while(1){

    //job_body
    task2();  
    //cekanje na sledecu aktivaciju 
    wait_next_activation();

    
  

   }
}


int proces3(intervals int3){

    //inicijalizacija
    //intervals int1=*(intervals*)arg;
    int res;
    //startovanje tajmera
    res = start_periodic_timer(int3.offs,int3.period);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }


   int pmax = sched_get_priority_max(SCHED_FIFO);
   struct sched_param param; 
   param.sched_priority = pmax-int3.offs; 
   sched_setscheduler(0, SCHED_FIFO, &param);
 

    
    while(1){

    //job_body
    task3();  
    //cekanje na sledecu aktivaciju 
    wait_next_activation();

    
  

   }
}






void msg_error()
{
   printf("Fork nije uspio");

}









void print_child_status (int status) {
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




int main(){
   
  
    pid_t child1,child2,child3;
    int status1,status2,status3;


    sigset_t sig_alrm;
    sigemptyset(&sig_alrm);
    sigaddset(&sig_alrm,SIGALRM);
    sigprocmask(SIG_BLOCK,&sig_alrm,NULL);

    
    child1 = fork();
    if (child1 < 0) {
        perror("Fork");
        return -1;
    }
    if (child1 == 0) {
		// child code
		proces1(inter1);
        return 31;
		// child code
    }
    
   
    
    child2 = fork();
    if (child2 < 0) {
        perror("Fork");
        return -1;
    }
    if (child2 == 0) {
		// child code
		proces2(inter2);
        return 32;
		// child code
    }

    
    child3 = fork();
    if (child3 < 0) {
        perror("Fork");
        return -1;
    }
    if (child3 == 0) {
		// child code
		proces3(inter3);
        return 33;
		// child code
    }

     // Parent code    

	int pmax = sched_get_priority_max(SCHED_FIFO);
	struct sched_param param; 
	param.sched_priority = pmax; 
	sched_setscheduler(0, SCHED_FIFO, &param);
    
    waitpid(child3,&status3,0);
    print_child_status(status3);
    waitpid(child2,&status2,0);
    print_child_status(status2);
    waitpid(child1,&status1,0);
    print_child_status(status1);
    
    

    return 0;











}
















