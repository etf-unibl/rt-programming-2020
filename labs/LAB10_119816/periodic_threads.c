#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

struct periodic_info {
    int id;
    int sig;
	sigset_t alarm_sig;
};

pthread_mutex_t mutex;

void task1(void)
{
  int i,j;
 
  for (i=0; i<4; i++) {
    for (j=0; j<250000; j++) ;
    printf("1");
    fflush(stdout);
  }
}

void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<2500000; j++) ;
    printf("2");
    fflush(stdout);
  }
}


static void thread_control_handler(int n, siginfo_t* siginfo, void* sigcontext) {
    // wait time out
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
}

// suspend a thread for some time
void thread_suspend(int tid, int time) {
    struct sigaction act;
    struct sigaction oact;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = thread_control_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&act.sa_mask);
    pthread_mutex_init(&mutex, 0);
    if (!sigaction(SIGURG, &act, &oact)) {
        pthread_mutex_lock(&mutex);
        kill(tid, SIGURG);
        usleep(time);
        pthread_mutex_unlock(&mutex);
    }
}

static void wait_period(struct periodic_info *info,int Signal)
{
	int sig;
	
	if(info->sig==Signal){
	thread_suspend(info->id,59000);
	   // sigwait(&(info->alarm_sig), &sig);
    	    
	}
	else if(info->sig==Signal){
	 thread_suspend(info->id,79000);
	 //sigwait(&(info->alarm_sig), &sig);  
	 
	}
	else printf("please dont print");
}

static int start_periodic(unsigned int period, struct periodic_info *info)
{
	int err_code;
	struct itimerval value;
    static int next_sig;

    if (next_sig == 0)
		next_sig = SIGRTMIN;
	/* Check that we have not run out of signals */
	if (next_sig > SIGRTMAX)
		return -1;
	info->sig = next_sig;
	next_sig++;
	/* Create the signal mask that will be used in wait_period */
	sigemptyset(&(info->alarm_sig));
	sigaddset(&(info->alarm_sig), info->sig);
//	printf("value of sigwait %d\n",info->sig);

	// Setting the timer to go off after the first period and then repetitively 
	value.it_value.tv_sec = period / 1000000;
	value.it_value.tv_usec = period % 1000000;
	value.it_interval.tv_sec = period / 1000000;
	value.it_interval.tv_usec = period % 1000000;
	
	sigemptyset(&info->alarm_sig);
	sigaddset(&info->alarm_sig, SIGALRM); 
	sigaddset(&info->alarm_sig, SIGUSR1);
	sigprocmask(SIG_BLOCK, &info->alarm_sig, NULL); 

	
	err_code = setitimer(ITIMER_REAL, &value, NULL);
	if (err_code != 0)
		perror("Failed to set timer");
	return err_code;
}


static void *thread_func_1(void *arg)
{
	struct periodic_info info;
	info.id=1;
	start_periodic(60000, &info);
int	signal_1=info.sig;
	while (1) {
		task1();
		wait_period(&info,signal_1);
	}
	return NULL;
}

static void *thread_func_2(void *arg)
{
	struct periodic_info info;
    info.id=2;
	start_periodic(80000, &info);
    int	signal_2=info.sig;
	while (1) {
		task2();
		wait_period(&info,signal_2);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int err_code;
	pthread_t thread_1;
	pthread_t thread_2;
	sigset_t alarm_sig;

	sigemptyset(&alarm_sig);
	sigaddset(&alarm_sig, SIGALRM);
	sigprocmask(SIG_BLOCK, &alarm_sig, NULL);

	pthread_create(&thread_1, NULL, thread_func_1, NULL);
	pthread_create(&thread_2, NULL, thread_func_2, NULL);
//	sleep(1);
	
    if (err_code=((pthread_join (thread_1, NULL))!= 0 && pthread_join (thread_2, NULL)!=0))
    printf ("Error joining thread , error=%d\n", err_code);


	
	
	
	return 0;
}
