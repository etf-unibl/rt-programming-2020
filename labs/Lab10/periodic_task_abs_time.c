#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h> 

static struct timespec r[2];
static int PERIOD[2];

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

void* thread1(void *arg)
{
    int res;
    res = start_periodic_timer(0, 60000,0);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }
    while(1)
    {
        task1();
        wait_next_activation(0);
    }
    return NULL;
}

void* thread2(void *arg)
{
    int res;
    res = start_periodic_timer(0, 80000,1);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }
    while(1)
    {
        task2();
        wait_next_activation(1);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thr;
    pthread_create(&thr,NULL,thread1,0);
    pthread_create(&thr,NULL,thread2,0);
    
    pthread_join(thr,NULL);
    return 0;
}
