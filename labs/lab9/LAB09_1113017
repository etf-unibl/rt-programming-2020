// Kompajlirati sa 'gcc thread_template.c -lpthread -lrt -Wall'

   #include <stdlib.h>
   #include <stdio.h>
   #include <sys/mman.h> // potrebno za mlockall()
   #include <unistd.h>  //  potrebno za sysconf(int name);
   #include <malloc.h>
   #include <sys/time.h>  
   #include <sys/resource.h>  
   #include <pthread.h>
   #include <limits.h>
   #include <time.h>

   #define PRE_ALLOCATION_SIZE (10*1024*1024) /* 100MB pagefault free buffer */
   #define MY_STACK_SIZE       (100*1024)      /* 100 kB dodatak za stek */

   static pthread_mutex_t mtx;
   static pthread_mutexattr_t mtx_attr; 

/*
       Currently, Linux supports the following "normal" (i.e., non-real-
       time) scheduling policies as values that may be specified in policy:
       SCHED_OTHER   the standard round-robin time-sharing policy;
       SCHED_BATCH   for "batch" style execution of processes; and
       SCHED_IDLE    for running very low priority background jobs.

       Various "real-time" policies are also supported:
       SCHED_FIFO    a first-in, first-out policy; and
       SCHED_RR      a round-robin policy.

*/ 

    struct thread_param
    {
    struct timespec ts;
    int priority;
    int stack_size; 
    int increment;
    };  

   static int shared_val; 
   static int policy=SCHED_RR;
   
   static void setprio(int prio, int sched)
   {
   	struct sched_param param; 
   	param.sched_priority = prio;
   	if (sched_setscheduler(0, prio, &param) < 0)
   		perror("sched_setscheduler");
   }
   
   void show_new_pagefault_count(const char* logtext, 
   			      const char* allowed_maj,
   			      const char* allowed_min)
   {
	   // ispis pagefaultova!
   	static int last_majflt = 0, last_minflt = 0;
   	struct rusage usage;
   	getrusage(RUSAGE_SELF, &usage);
   	printf("%-30.30s: Pagefaults, Major:%ld (Allowed %s), " \
   	       "Minor:%ld (Allowed %s)\n", logtext,
   	       usage.ru_majflt - last_majflt, allowed_maj,
   	       usage.ru_minflt - last_minflt, allowed_min);
   	last_majflt = usage.ru_majflt; 
   	last_minflt = usage.ru_minflt;
   }

   /*************************************************************/
   /* Funkcija programske niti */
   static void *non_res_thread_fn(struct thread_param *parameters)
   { 
   	setprio(policy, parameters->priority);
    for (int i=0;10>i;i++) {
    printf("NIT %d\n",parameters->priority); 
   	clock_nanosleep(CLOCK_REALTIME, 0, &parameters->ts, NULL);
    fflush(stdout);
    }
   	return NULL;
   }

static void *resource_thread_fn(struct thread_param *parameters)
   {
setprio(policy,parameters->priority);
pthread_mutex_lock(&mtx); 
for (int i=0;10>i;i++) {
printf("NIT:%d -  shared value: %d\n",parameters->priority, shared_val);
shared_val=shared_val+parameters->increment; 
fflush(stdout);
clock_nanosleep(CLOCK_REALTIME, 0, &parameters->ts, NULL);
}
pthread_mutex_unlock(&mtx);  
   	return NULL;
   }
   /*************************************************************/
   static void error(int at)
   {
   	/* Ispisi gresku i izadji */
   	fprintf(stderr, "Some error occured at %d", at);
   	exit(1);
   }

static pthread_t start_thread(void *thread_function, struct thread_param  *parameters)
   {
   	pthread_t thread;
   	pthread_attr_t attr;
   	/* inicijalizacija programske niti */
   	if (pthread_attr_init(&attr))
   		error(1);
   	/* inicijalizacija memorije potrebne za stek */
   	if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + parameters->stack_size) || parameters->stack_size<0)
   		error(2);
   	/* kreiranje programske niti */
   	pthread_create(&thread, &attr, thread_function,  parameters);
   	return thread;
   }
static void configure_malloc_behavior(void)
   {
   	/* konfiguracija allociranja memorije */
   	if (mlockall(MCL_CURRENT | MCL_FUTURE))
   		perror("mlockall failed:");
   	/* sbrk nema nazad */
   	mallopt(M_TRIM_THRESHOLD, -1);
   	/* iskljuci mmap */
   	mallopt(M_MMAP_MAX, 0);
   }
static void reserve_process_memory(int size)
   {
	   // rezervisanje memorije, guranje svega u RAM
   	int i;
   	char *buffer;
   	buffer = malloc(size);
   	for (i = 0; i < size; i += sysconf(_SC_PAGESIZE)) {
   		buffer[i] = 0;
   	}
   	free(buffer);
   }
   int main(int argc, char *argv[])
   {
   	configure_malloc_behavior();

   	reserve_process_memory(PRE_ALLOCATION_SIZE);

    pthread_mutexattr_init(&mtx_attr);
    pthread_mutexattr_setprotocol(&mtx_attr,PTHREAD_PRIO_PROTECT);
    pthread_mutex_init(&mtx,&mtx_attr);  

    struct thread_param *thread1=malloc(sizeof(struct thread_param)); 
    struct thread_param *thread2=malloc(sizeof(struct thread_param));
    struct thread_param *thread3=malloc(sizeof(struct thread_param)); 

    struct timespec tx;
   	tx.tv_sec =3;
   	tx.tv_nsec = 0;
    struct timespec tmp;
   	tmp.tv_sec = 0;
   	tmp.tv_nsec =1;
    
    /* L  */ 
    thread1->ts=tx;
    thread1->priority=1;
    thread1->stack_size=1024*5; 
    thread1->increment=1;

   	/* M  */ 
    thread2->ts=tx;
    thread2->priority=2;
    thread2->stack_size=1024*5; 
    thread2->increment=1;


    /* H  */ 
    thread3->ts=tx;
    thread3->priority=3;
    thread3->stack_size=1024*5; 
    thread3->increment=1;

    start_thread(resource_thread_fn,thread1);  
    clock_nanosleep(CLOCK_REALTIME, 0, &tmp, NULL); // obezbjedimo da uvijek prva nit zauzme mutex 
    start_thread(resource_thread_fn,thread3);  
    start_thread(non_res_thread_fn, thread2);  

    pthread_mutexattr_destroy(&mtx_attr); 
    pthread_mutex_destroy(&mtx); 
    free(thread1);
    free(thread2);
    free(thread3);
    sleep(10); 
    fflush(stdout);
   	printf("Press <ENTER> to exit\n");
   	getchar();
   	return 0;
   }
