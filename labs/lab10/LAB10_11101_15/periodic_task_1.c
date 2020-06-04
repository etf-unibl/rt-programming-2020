#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
//arm-linux-gnueabihf-gcc periodic_task_1.c -std=gnu99 -Wall -o periodic_task_1
//gcc periodic_task_1.c -std=gnu99 -Wall -o periodic_task_1
static int signal_entry = 0;
#define wait_next_activation pause
static void sighand(int s)
{
	
}
int start_periodic_timer(uint64_t offs, int period)
{
    struct itimerval t;

    t.it_value.tv_sec = offs / 1000000;
    t.it_value.tv_usec = offs % 1000000;
    t.it_interval.tv_sec = period / 1000000;
    t.it_interval.tv_usec = period % 1000000;

    signal(SIGALRM, sighand);

    return setitimer(ITIMER_REAL, &t, NULL);
}

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


int main(int argc, char *argv[])
{
    int res;
    int ct1=3;
    int ct2=4;
   
    res = start_periodic_timer(2000000, 20000);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }	
	//tajmer se aktivira  svakih 20ms, kada se aktivira tri puta krece task1, a cetri puta task2
    while(1) {
    	wait_next_activation();
		if(ct1==3){
			task1();
			ct1=0;
		}
		if(ct2==4){
			task2();
			ct2=0;
		}
			
		ct1++;
		ct2++;
		
    }
  /* Drugi nacin, nema provjere u if petljama.
   while(1) {
	task1();
	task2();
    wait_next_activation();
	wait_next_activation();
	wait_next_activation();
	task1();	
	wait_next_activation();
	task2();
	wait_next_activation();
	wait_next_activation();
	task1();
	wait_next_activation();
	wait_next_activation();
	task2();
	wait_next_activation();
	task1();
	wait_next_activation();
	wait_next_activation();
	wait_next_activation();
    }
	*/
    return 0;
}

   

