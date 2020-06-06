#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>


/*Parametri za niti*/
typedef struct{
    struct timespec r;
    int period;
} thread_param;



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

/*Funkcija za nit 1 koja izvrsava zadatak 1*/
void* thread_function1(void* pParam)
{
    thread_param param = *(thread_param*)pParam;

    while(1)
    {
        task1();
        wait_next_activation(&param.r, param.period);  
    }
    return 0;    
}

/*Funkcija za nit 2 koja izvrsava zadatak 2*/
void* thread_function2(void* pParam)
{
    thread_param param = *(thread_param*)pParam;

    while(1)
    {
        task2();
        wait_next_activation(&param.r, param.period);
    }
    return 0;    

}
    


int main(int argc, char *argv[])
{
    int res;

    pthread_t thread1;
    pthread_t thread2;
  
    /*Parametri za niti*/
    thread_param param1;
    thread_param param2;


    /*U ovoj funkciji setuje parametre za prvu nit*/
    res = start_periodic_timer(2000000, 60000, &param1.r, &param1.period);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }

    /*U ovoj funkciji setuje parametre za drugu nit*/
    res = start_periodic_timer(2500000, 80000, &param2.r, &param2.period);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }
    
    /*Kreiranje prve niti*/
    if(pthread_create(&thread1, NULL, thread_function1, (void*)&param1))
    {
        printf("\nNeuspjesno kreiranje niti 1!");
        return -1;
    }

    /*Kreiranje druge niti*/
    if(pthread_create(&thread2, NULL, thread_function2, (void*)&param2))
    {
        printf("\nNeuspjesno kreiranje niti 2!");
        return -1;
    }
    
    getchar();
    
    return 0;
}
