#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

static struct timespec r;
static int period;
static pthread_mutex_t cs_mutex;

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

void* routine(void* param)
{
        int i = *(int*)param;
        while(1)
        {
                pthread_mutex_lock(&cs_mutex);
                
	        if(i==1)
		        task1();
	        else if(i==2)
		        task2();
	  
	         wait_next_activation();
	  
	        pthread_mutex_unlock(&cs_mutex);
        }
        return NULL;
}


int main(int argc, char *argv[])
{
    int res, res1, res2;
	res1=1;
	res2=2;
         pthread_t thread1;
        pthread_t thread2;
  
	pthread_mutex_init(&cs_mutex, NULL);
    
        res = start_periodic_timer(2000000, 60000);
        if (res < 0) 
        {
                perror("Start Periodic Timer");
                return -1;                
        }   

        res = start_periodic_timer(2500000, 80000);
        if (res < 0) 
        {
                perror("Start Periodic Timer");
                return -1;
        }
    
    if(pthread_create(&thread1, NULL, routine, (void*)&res1))
    {
                printf("\nNeuspjesno kreiranje niti 1!");
                return -1;
    }

    if(pthread_create(&thread2, NULL, routine, (void*)&res2))
    {
        printf("\nNeuspjesno kreiranje niti 2!");
        return -1;
    }
    
    getchar();
	
	pthread_mutex_destroy(&cs_mutex);
    
    return 0;
}
