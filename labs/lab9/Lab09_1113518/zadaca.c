// Kompajlirati sa 'gcc thread_template.c -lpthread -lrt -Wall'
   #include <stdlib.h>
   #include <stdio.h>
   #include <sys/mman.h> // potrebno za mlockall()
   #include <unistd.h> // potrebno za sysconf(int name);
   #include <malloc.h>
   #include <sys/time.h> // potrebno za getrusage
   #include <sys/resource.h> // potrebno za getrusage
   #include <pthread.h>
   #include <limits.h>
   
   #define PRE_ALLOCATION_SIZE (10*1024*1024) /* 100MB pagefault free buffer */
   #define MY_STACK_SIZE       (100*1024)      /* 100 kB dodatak za stek */
   static pthread_mutex_t mtx;
   static pthread_mutexattr_t mtx_attr;
   
   static int shared_val = 0;
   
   struct argument{
	   int sleep_time;
	   int prio;
	   int is_print;
	   int increment;
   };
   
   static void setprio(int prio, int sched)
   {
   	struct sched_param param;
   	// podesavanje prioriteta i schedulera
   	// vise o tome kasnije
   	param.sched_priority = prio;
   	if (sched_setscheduler(0, sched, &param) < 0)
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
   
   static void prove_thread_stack_use_is_safe(int stacksize, int do_log)
   {
	   //gurni stek u RAM
   	volatile char buffer[stacksize];
   	int i;
   
   	for (i = 0; i < stacksize; i += sysconf(_SC_PAGESIZE)) {
   		/* "Touch" za cijeli stek od programske niti */
   		buffer[i] = i;
   	}
	 if(do_log)
		show_new_pagefault_count("Caused by using thread stack", "0", "0");
   }
   
   /*************************************************************/
   /* Funkcija programske niti */
   static void *resource_thread_fn(void *args){
		struct argument arg = *(struct argument*)args;
		struct timespec ts;
		ts.tv_sec = arg.sleep_time;
		ts.tv_nsec = 0;
   
		setprio(arg.prio, SCHED_RR);
		
   
		//<do your RT-thing here>
		
		int i;
		
		printf("Thread sa prioritetom %d jos ne koristi djeljeni resurs.\n", arg.prio);
		for(i = 0; i < 10; i++){
		}
		pthread_mutex_lock(&mtx);
		for(i = 10; i < 20; i++){
			shared_val += arg.increment;
			if(arg.is_print == 0){
				printf("Thread sa prioritetom %d poveco vrijednost djeljenog resursa sa %d na %d.\n", arg.prio, shared_val - arg.increment, shared_val);
			}
		}
		printf("Thread sa prioritetom %d gotov sa djeljeni resurs.\n", arg.prio);
		pthread_mutex_unlock(&mtx);
		for(i = 20; i < 30; i++){
		}
		
   
		/* spavati 30 sekundi */
		clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
		//printf("shared value%i\n", shared_val);
		return NULL;
   }
   
   /* Funkcija programske niti */
   static void *non_res_thread_fn(void *args){
		struct argument arg = *(struct argument*)args;
		struct timespec ts;
		ts.tv_sec = arg.sleep_time;
		ts.tv_nsec = 0;
   
		setprio(arg.prio, SCHED_RR);
		
   
		//<do your RT-thing here>
		
		int i;
		
		for(i = 0; i < 10; i++){
			if(arg.is_print == 0){
				printf("Thread sa prioritetom %d ne koristi djeljeni resurs.\n", arg.prio);
			}
		}
   
		/* spavati 30 sekundi */
		clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
		//printf("shared value%i\n", shared_val);
		return NULL;
   }
   
   /*************************************************************/
   
   static void error(int at)
   {
   	/* Ispisi gresku i izadji */
   	fprintf(stderr, "Some error occured at %d", at);
   	exit(1);
   }
   
   static pthread_t start_rt_thread(int type, int stack_size, void* args)
   {
   	pthread_t thread;
   	pthread_attr_t attr;
   
   	/* inicijalizacija programske niti */
   	if (pthread_attr_init(&attr))
   		error(1);
   	/* inicijalizacija memorije potrebne za stek */
   	if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + stack_size))
   		error(2);
   	/* kreiranje programske niti */
   	if(type == 0){
		pthread_create(&thread, &attr, resource_thread_fn, args);
	}else{
		pthread_create(&thread, &attr, non_res_thread_fn, args);
	}
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
   
	//<do your RT-thing>
   
	pthread_mutexattr_init(&mtx_attr);
	pthread_mutexattr_setprotocol(&mtx_attr, PTHREAD_PRIO_PROTECT);
	pthread_mutex_init(&mtx, &mtx_attr);
	//pthread_mutex_init(&mtx, NULL);
   
	struct argument H, M, L;
	H.sleep_time = 1; H.prio = sched_get_priority_max(SCHED_RR);
	M.sleep_time = 1; M.prio = (sched_get_priority_max(SCHED_RR) - sched_get_priority_min(SCHED_RR)) / 2;
	L.sleep_time = 1; L.prio = sched_get_priority_min(SCHED_RR);
	H.is_print = 0; H.increment = 2;
	M.is_print = 0; M.increment = 0;
	L.is_print = 0; L.increment = 1;
	
	start_rt_thread(0, MY_STACK_SIZE, (void*)&L);
	start_rt_thread(0, MY_STACK_SIZE, (void*)&H);
	start_rt_thread(1, MY_STACK_SIZE, (void*)&M);
	
   	printf("Press <ENTER> to exit\n");
   	getchar();
   	pthread_mutexattr_destroy(&mtx_attr);
   	pthread_mutex_destroy(&mtx);
   
   	return 0;
   }
   
