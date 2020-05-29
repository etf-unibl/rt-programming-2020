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
   #include <stdbool.h>
   
   #define PRE_ALLOCATION_SIZE (10*1024*1024) /* 10 MB pagefault free buffer */
   #define MY_STACK_SIZE       (100*1024)      /* 100 kB dodatak za stek */
   static pthread_mutex_t mutex;
   static pthread_mutexattr_t mutex_attr;
  
   static int shared_val = 0;     //dijeljeni resurs

   /*Struktura za argument niti L i H, ima dodatni clan add_shared_val
     kojim uvecava shared_val */
   typedef struct {
    int sleep_time;         //vrijeme spavanja        
    int priority;           //prioritet
    int add_stack_size;     //dodatna velicina steka
    bool print;             //ispisi sta nit radi na ekran (true/false) 
    int add_shared_val;     //broj kojim se uvecava dijeljeni resurs
   } arg_struct_LH;
    
   /*Struktura za argument niti M 
     (nema clan add_shared_val jer M nema dijeljeni resurs*/
   typedef struct {
    int sleep_time;         //vrijeme spavanja       
    int priority;           //prioritet
    int add_stack_size;     //dodatna velicina steka
    bool print;             //ispisi sta nit radi na ekran (true/false) 
   } arg_struct_M;



   static void setprio(int prio, int sched)
   {
   	struct sched_param param;

   	// podesavanje prioriteta i schedulera
   	param.sched_priority = prio;
   	if (sched_setscheduler(0, sched, &param) < 0)
   		perror("sched_setscheduler");
   }



   void show_new_pagefault_count(const char* logtext, const char* allowed_maj, const char* allowed_min)
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
   /* Funkcija L i H programske niti */
    static void *resource_thread_fn(void *args)
    {
        arg_struct_LH arg = *(arg_struct_LH*)args;             
        char tmp; 
        int tmp2;                                             
        int i;                                               
        struct timespec ts;
        ts.tv_sec = arg.sleep_time;
   	    ts.tv_nsec = 0;
   	   
        
        /*tmp sluzi da znamo koji tred se trenutno izvrsava)*/
        if(arg.priority == 1)
            tmp = 'L';
        else 
            if(arg.priority == 3)
                tmp = 'H';
        /*setujemo prioritet*/
    	setprio(arg.priority, SCHED_RR);
        
        if(pthread_mutex_lock(&mutex))
            printf("\nNeuspjesno zakljucavanje treda %c.\n",tmp);
        for(i = 0; i < 1000; i++)
        {   //uvecava se dijeljeni resurs
            shared_val = shared_val + arg.add_shared_val;

            if(arg.print == true)//ako treba da se printa na ekran
            {
                printf("%c%d ", tmp, shared_val);
            }
                                                                        
        }
        printf("\n");
        if(pthread_mutex_unlock(&mutex))
            printf("\nNeuspjesno otkljucavanje treda %c.\n",tmp);

    	
   	    /* spavati /arg.sleep_time/ sekundi */
   	    clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);

        return NULL;        
    }
   


    /* Funkcija M programske niti */
    static void *non_res_thread_fn(void *args)
    {
        arg_struct_M arg = *(arg_struct_M*)args;        
        int i;             
                          
        struct timespec ts;
        ts.tv_sec = arg.sleep_time;
   	    ts.tv_nsec = 0;
   
   	    setprio(arg.priority, SCHED_RR);
        printf("\n\nNIT: M\n\n");
        for(i = 0; i < 1000; i++)
        {   
            if(arg.print == true) //ako treba da se printa na ekran
            {
                printf("M ");                         
            }                                             
        }
        printf("\n");
   
   	  
   	    /*Spavati /arg.sleep_time/ sekundi */
   	    clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
        
   	    return NULL;
    }    
    


   /*************************************************************/
   static void error(int at)
   {
   	/* Ispisi gresku i izadji */
   	fprintf(stderr, "Some error occured at %d", at);
   	exit(1);
   }
  


   pthread_t start_rt_thread(int which_thr, int sleep_time, 
                        int priority, int add_stack_size, bool print, int add_shared_val)
   {
   	pthread_t thread;
   	pthread_attr_t attr;
   
   	/* inicijalizacija programske niti */
   	if (pthread_attr_init(&attr))
   		error(1);

    /* inicijalizacija memorije potrebne za stek */
    if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + add_stack_size) || add_stack_size<0)
        error(2);


   	/* kreiranje programske niti, provjeravamo o kom se tredu radi
       ako je which_thr = 1, tada se kreira tred H ili L sa djeljenim resursnom*/
    if(which_thr == 1)
    {
        arg_struct_LH param;
        param.sleep_time = sleep_time;
        param.priority = priority;
        param.add_stack_size = add_stack_size;
        param.print = print;
        param.add_shared_val = add_shared_val;
   	    pthread_create(&thread, &attr, resource_thread_fn, (void*)&param);  
    }
    else  /*Ako je which_thr = 2, tada se kreira tred M bez dijeljenog resursa */
        if(which_thr == 2)
        {
            arg_struct_M param;
            param.sleep_time = sleep_time;
            param.priority = priority;
            param.add_stack_size = add_stack_size;
            param.print = print;
            pthread_create(&thread, &attr, non_res_thread_fn, (void*)&param);
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
    int tmp;
    /*Inicijalizacija mutex atributa*/
    tmp = pthread_mutexattr_init(&mutex_attr);
    if (tmp) {
		printf("\nGreska kod kreiranja mutex attr.");
		return -1;
	}
    /*Setovanje trazenog protokola*/
    tmp = pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_PROTECT);
    if (tmp) {
		printf("\nGreska kod postavljanja protokola u mutex attr!");
		return -1;
	}    
    /*Inicijalizacija mutexa*/
    tmp = pthread_mutex_init(&mutex, &mutex_attr);
    if (tmp) {
		printf("\nGreska kod kreiranja mutexa!");
		return -1;
	} 
    

                      //start_rt_thread(which_thr, sleep_time, priority, add_stack_size, print, add_shared_val)               
    pthread_t threadL = start_rt_thread(   1,           2,        1,     MY_STACK_SIZE,  true,        2       );
    pthread_t threadH = start_rt_thread(   1,           2,        3,     MY_STACK_SIZE,  true,        5       );
    pthread_t threadM = start_rt_thread(   2,           2,        2,     MY_STACK_SIZE,  true,        0       );

   	configure_malloc_behavior();                                         
   
   	reserve_process_memory(PRE_ALLOCATION_SIZE);                         
  
    /*Unistavanje mutex atributa*/
    tmp = pthread_mutexattr_destroy(&mutex_attr);
    if (tmp) {
		printf("\nGreska pri unistavanju mutex attr.");
		return -1;
	}    
    /*Unistavanje mutexa*/
    tmp = pthread_mutex_destroy(&mutex);
	if (tmp) {
		printf("\nGreska pri unistavanju mutex-a.");
		return -1;
	}
    
   	printf("\nPress <ENTER> to exit\n");
   	getchar();
        
   	return 0;
   }
   
