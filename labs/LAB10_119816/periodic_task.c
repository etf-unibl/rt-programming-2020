#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

struct periodic_info {
	sigset_t alarm_sig;
};

static int make_periodic(unsigned int period, struct periodic_info *info)
{
	int err_code;
	struct itimerval value;

	/* Block SIGALRM in this thread */
	sigemptyset(&(info->alarm_sig));
	sigaddset(&(info->alarm_sig), SIGALRM);
	pthread_sigmask(SIG_BLOCK, &(info->alarm_sig), NULL);

	// Setting the timer to go off after the first period and then repetitively 
	value.it_value.tv_sec = period / 1000000;
	value.it_value.tv_usec = period % 1000000;
	value.it_interval.tv_sec = period / 1000000;
	value.it_interval.tv_usec = period % 1000000;
	err_code = setitimer(ITIMER_REAL, &value, NULL);
	if (err_code != 0)
		perror("Failed to set timer");
	return err_code;
}

static void wait_period(struct periodic_info *info)
{
	int sig;

	/* Wait for the next SIGALRM */
	sigwait(&(info->alarm_sig), &sig);
}

void task1(void)
{
  int i,j;
 
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("0");
    //printf("1");
    fflush(stdout);
  }
}

void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    //printf("2");
    printf("|");
    fflush(stdout);
  }
}


int main(int argc, char *argv[])
{
    
	sigset_t alarm_sig;
	inc count_1=0;
	int count_2=0;
	sigemptyset(&alarm_sig);
	sigaddset(&alarm_sig, SIGALRM);
	sigprocmask(SIG_BLOCK, &alarm_sig, NULL);

   struct periodic_info info;
    //Minimum common period is 20ms for 60ms and 80ms tasks   	
   	make_periodic(20000, &info);
   	while(1){
		wait_period(&info);
		if(count_1==3){ task1(); count_1=0;}
		if (count_2==4) { task2(); count_2=0;}
		count_1++; 
        	count_2++;		
	}
    
	return 0;
}




