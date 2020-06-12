#include <sys/types.h>
 
#include <sys/wait.h>
 
#include <unistd.h>
 
#include <stdio.h>
 
#include <sched.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>
 
static sigset_t sigset;  
 
 
pid_t child;
 
int status;
 
 
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
 
// periodicni zadatak na 40 milisekundi
 
void task1(void)
{
child = fork();
 
    if (child < 0) {
 
        perror("Fork");
 
        return -1;
 
    }
 
    if (child == 0) {
    int i,j;
    int pmin = sched_get_priority_min(SCHED_FIFO);
 
    struct sched_param param;
 
    param.sched_priority = pmin + 30;
 
    sched_setscheduler(0, SCHED_FIFO, &param);
    for (i=0; i<4; i++) {
        for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
    }
    return 1;
    }
}
 
// periodicni zadatak na 80 milisekundi
void task2(void)
{
child = fork();
 
    if (child < 0) {
 
        perror("Fork");
 
        return -1;
 
    }
 
    if (child == 0) {
    int i,j;
    int pmin = sched_get_priority_min(SCHED_FIFO);
 
    struct sched_param param;
 
    param.sched_priority = pmin + 20;
 
    sched_setscheduler(0, SCHED_FIFO, &param);
    for (i=0; i<6; i++) {
        for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
    }
    return 2;
    }
}
 
// periodicni zadatak na 120 milisekundi
void task3(void)
{
    child = fork();
 
    if (child < 0) {
 
        perror("Fork");
 
        return -1;
 
    }
 
    if (child == 0) {
 
        int i,j;
 
        int pmin = sched_get_priority_min(SCHED_FIFO);
 
        struct sched_param param;
 
        param.sched_priority = pmin + 10;
 
        sched_setscheduler(0, SCHED_FIFO, &param);
        for (i=0; i<6; i++) {
         for (j=0; j<100000; j++) ;
        printf("3");
        fflush(stdout);
        }
        return 3;
        }
 
}
 
 
int main(int argc, char *argv[])
 
{
    int task1counter=0,task2counter=0,task3counter=0;
    int pmin = sched_get_priority_min(SCHED_FIFO);
    struct sched_param param;
    param.sched_priority = pmin;
    sched_setscheduler(0, SCHED_FIFO, &param);
 
    start_periodic_timer(2000000, 20000);
    while(1)
    {
        wait_next_activation();
        if (task1counter==2) { task1(); task1counter=0;}
        if (task2counter==4) { task2(); task2counter=0;}    
        if (task3counter==6) { task3(); task3counter=0;}
        task1counter++;
        task2counter++;
        task3counter++;
    }
 
    return 0;
 
}
