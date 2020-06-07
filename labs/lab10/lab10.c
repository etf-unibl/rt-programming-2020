#include <sys/time.h>

#include <time.h>

#include <signal.h>

#include <unistd.h>

#include <stdint.h>

#include <stdio.h>



//arm-linux-gnueabihf-gcc lab10.c -std=gnu99 -Wall -o lab10



static sigset_t sigset;  

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

	sigwait(&sigset, &dummy); 

}



int start_periodic_timer(uint64_t offs, int period)

{

    struct itimerval t;



    t.it_value.tv_sec = offs / 1000000;

    t.it_value.tv_usec = offs % 1000000;

    t.it_interval.tv_sec = period / 1000000;

    t.it_interval.tv_usec = period % 1000000;

	sigemptyset(&sigset); // inicijalizacija

	sigaddset(&sigset, SIGALRM); // ubacivanje SIGALRM u set

	sigprocmask(SIG_BLOCK, &sigset, NULL); // podesavanje koji se skup

										   // signala prati

    return setitimer(ITIMER_REAL, &t, NULL);

}





int main(int argc, char *argv[])

{

    int task1counter=0,task2counter=0;

    start_periodic_timer(2000000, 20000);

	while(1)

	{

		wait_next_activation();

		if(task1counter==3){ task1(); task1counter=0;}

		if (task2counter==4) { task2(); task2counter=0;}

		task1counter++; 

        task2counter++;		

	}



    return 0;

}
