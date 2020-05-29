#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>		// potrebno za mlockall()
#include <unistd.h>		// potrebno za sysconf(int name);
#include <malloc.h>
#include <sys/time.h>		// potrebno za getrusage
#include <sys/resource.h>	// potrebno za getrusage
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>		// bool tip!
#include <unistd.h>
#include <errno.h>		// Potrebno za provjeru nemogucnosti zakljucavanja Mutex-a!

#define PRE_ALLOCATION_SIZE (10*1024*1024) /* 10MB pagefault free buffer */
#define MY_STACK_SIZE       (100*1024)      /* 100 kB dodatak za stek */
//sleep tome for individual threads
#define thread1_sleep 100
#define thread2_sleep 200
#define thread3_sleep 300
  
//Thread structure
    typedef struct{
        char print;
        int thread_priority;
        int add_resource;
        int stack_size_add;
        double thread_sleep;
        void *(*my_rt_thread) (void *structure);
    } thread_struct;

    static int shared_val = 0;
    
    static pthread_mutex_t mtx;

 
 
/* Funkcija za postavljanje prioriteta programske niti! */ 
    static void setprio (int prio, int sched){
    
    struct sched_param param;
    // podesavanje prioriteta i schedulera
    param.sched_priority = prio;
    if (sched_setscheduler (0, sched, &param) < 0)
        perror ("sched_setscheduler");
    }

/* Smjestanje steka programske niti u RAM! */ 
    static void prove_thread_stack_use_is_safe (int stacksize, int do_log){
        volatile char buffer[stacksize];
        int i;
        for (i = 0; i < stacksize; i += sysconf (_SC_PAGESIZE))
        {
	        /* "Touch" za cijeli stek od programske niti */ 
	        buffer[i] = i;
        }
    }
    
/* Funkcija za ispis greske, i izlazak! */ 
    static void error (int at){
    /* Ispisi gresku i izadji */ 
    fprintf (stderr, "Some error occured at %d\n", at);
    exit (1);
    } 
//Function for converting from seconds to milliseconds 
    double convert_time (int Time){
        double sec;
        int a = Time / 1000;
        double b = Time % 1000;
        sec = (double) a + (b / 1000);
        return sec;
    }
    
//thread function with resources
    static void *thread_func_with_res (void *structure){
        int error;
        thread_struct thread = *(thread_struct *) structure;
        
        struct timespec ts;
        ts.tv_sec = (int) thread.thread_sleep;
        ts.tv_nsec = ((thread.thread_sleep - ts.tv_sec) * 1000) * 1000000;	// convert to nanosec
        setprio (thread.thread_priority, SCHED_RR);
        
        prove_thread_stack_use_is_safe (thread.stack_size_add, 1);
        clock_nanosleep (CLOCK_REALTIME, 0, &ts, NULL);
        
        while ((error = pthread_mutex_trylock (&mtx)) != 0){
            printf ("we are busy at work, waiting for resource\n");
            sleep (1);
        }
        if (thread.print){
            printf ("Thread with priority %d adding to resource\n",
	        thread.thread_priority);
        }
        shared_val += thread.add_resource;
        if (thread.print){
            printf("Thread with priority %d added to shared resource and the value is :%d\n",
	        thread.thread_priority, shared_val);
        }
        //How much the thread will sleep 
        float temp = 0;
        if (thread.thread_priority == 1){
            temp = thread1_sleep + thread3_sleep;
        }
        else{
            temp = thread3_sleep;
        }
        ts.tv_sec = (int) temp;
        ts.tv_nsec = ((temp - ts.tv_sec) * 1000) * 1000000;
        clock_nanosleep (CLOCK_REALTIME, 0, &ts, NULL);
        
        //Calculate the waiting time for thread
        if ((error = pthread_mutex_unlock (&mtx)) != 0){
            printf ("Error unlocking resource, error=%d\n", error);
            exit (error);
        }
        if (thread.print){
            printf ("Thread with priority %d finished\n", thread.thread_priority);
        }
        return NULL;
    }

    static void *thread_func_with_no_res (void *structure){
        
        thread_struct thread = *(thread_struct *) structure;
        struct timespec ts;
        ts.tv_sec = (int) thread.thread_sleep;
        ts.tv_nsec = ((thread.thread_sleep - ts.tv_sec) * 1000) * 1000000;
        clock_nanosleep (CLOCK_REALTIME, 0, &ts, NULL);
        
        setprio (thread.thread_priority, SCHED_RR);
        
        prove_thread_stack_use_is_safe (thread.stack_size_add, 1);
        
        if (thread.print){
            printf ("Thread with priority %d doing something\n",
	        thread.thread_priority);
        }
        clock_nanosleep (CLOCK_REALTIME, 0, &ts, NULL);
        if (thread.print){
            printf ("Thread with priority %d finished\n", thread.thread_priority);
        }
        return NULL;
    }


 
 
/* Kreiranje programskih niti! */ 
    static pthread_t start_rt_thread (thread_struct * structure){
        pthread_t thread;
        pthread_attr_t attr;
        /* inicijalizacija programske niti */ 
        if (pthread_attr_init (&attr))
            error (1);
        /* inicijalizacija memorije potrebne za stek */ 
        if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + structure->stack_size_add))
            error (2);
        /* kreiranje programske niti */ 
        pthread_create (&thread, &attr, structure->my_rt_thread,(void *) structure);
        return thread;
    }
    
/* Konfiguracija alociranja memorije */ 
    static void configure_malloc_behavior (void){
        if (mlockall (MCL_CURRENT | MCL_FUTURE))
            perror ("mlockall failed:");
        /* sbrk nema nazad */ 
        mallopt (M_TRIM_THRESHOLD, -1);
        /* iskljuci mmap */ 
        mallopt (M_MMAP_MAX, 0);
    }

/* Rezervisanje memorije, guranje 'svega' u RAM! */ 
    static void reserve_process_memory (int size){
        int i;
        char *buffer;
        buffer = malloc (size);
        for (i = 0; i < size; i += sysconf (_SC_PAGESIZE))
        buffer[i] = 0;
        free (buffer);
    }

    int main (){
        int error;
        if ((error = pthread_mutex_init (&mtx)) != 0){			
            printf ("Error initializing mutex, error=%d\n", error);
            exit (error);
        }
        configure_malloc_behavior ();
        reserve_process_memory (PRE_ALLOCATION_SIZE);
        
        //Create and inicialize threads
        //Inicialize structure 
	//Print.Priority,Add to shared res,Add to stack, Thread sleet, Finction pointer
        thread_struct thread_1 ={1, 1, 1, MY_STACK_SIZE, convert_time (thread1_sleep),*thread_func_with_res};
        thread_struct thread_2 ={1, 2, 2, MY_STACK_SIZE, convert_time (thread2_sleep),*thread_func_with_no_res};
        thread_struct thread_3 ={1, 3, 3, MY_STACK_SIZE, convert_time (thread3_sleep),*thread_func_with_res};
        
        //Start threads
        pthread_t thread_1 = start_rt_thread (&thread1);
        pthread_t thread_2 = start_rt_thread (&thread2);
        pthread_t thread_3 = start_rt_thread (&thread3);
  
        pthread_join (thread_1, NULL);
        pthread_join (thread_2, NULL);
        pthread_join (thread_3, NULL);
  
        printf ("Press <ENTER> to exit\n");
        getchar ();
        pthread_mutex_destroy (&mtx);
    
        return 0;

}
