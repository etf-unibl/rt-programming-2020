#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

//arm-linux-gnueabihf-gcc periodic_task_2.c -std=gnu99 -Wall -o periodic_task_2

// moguce je da postoji i vise setova
// u slucaju da je potrebno da se na razicit nacin
// ceka na razlicite setove signala
static sigset_t sigset;  // set signala
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


void wait_next_activation(void)
{
	int dummy;
	// ceka signal iz seta i u dummy upisuje
	// signal koji je dosao
	sigwait(&sigset, &dummy); 
	// ukoliko imamo vise signala onda bi ovdje
	// trebalo ispitati dummy
	// if (dummy == SIGALRM){}
}

int start_periodic_timer(uint64_t offs, int period)
{
    struct itimerval t;

    t.it_value.tv_sec = offs / 1000000;
    t.it_value.tv_usec = offs % 1000000;
    t.it_interval.tv_sec = period / 1000000;
    t.it_interval.tv_usec = period % 1000000;

    // signal(SIGALRM, sighand);
	sigemptyset(&sigset); // inicijalizacija
	sigaddset(&sigset, SIGALRM); // ubacivanje SIGALRM u set
	sigprocmask(SIG_BLOCK, &sigset, NULL); // podesavanje koji se skup
										   // signala prati

    return setitimer(ITIMER_REAL, &t, NULL);
}


int main(int argc, char *argv[])
{
    int res;
    int a=0,b=0;
    res = start_periodic_timer(2000000, 20000);
	/*res = start_periodic_timer(2000000, 80000);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }

    while(1) {
		task1();
		wait_next_activation();
		task2();
		wait_next_activation();
    }*/
	if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }
	// period taska1 je 60ms, tj treci put kada se pokrene brojac task1 se izvrsava
	while(1)
	{
		wait_next_activation();
		if(a==3){ task1(); a=0;}
		if (b==4) { task2(); b=0;}
		a++; b++;
		
		
		
	}

    return 0;
}
