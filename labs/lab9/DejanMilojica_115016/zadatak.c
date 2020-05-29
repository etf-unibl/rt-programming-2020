#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>     // potrebno za mlockall()
#include <unistd.h>       // potrebno za sysconf(int name);
#include <malloc.h>
#include <sys/time.h>     // potrebno za getrusage
#include <sys/resource.h> // potrebno za getrusage
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>      // bool tip!
#include <unistd.h>
#include <errno.h>		  // Potrebno za provjeru nemogucnosti zakljucavanja Mutex-a!

//Provjera podrzanosti mogucnosti "zastite" unutar mutex-a!
#ifndef _POSIX_THREAD_PRIO_PROTECT
#error "Sistem ne podrzava PTHREAD_PRIO_PROTECT!"
#endif
   
#define PRE_ALLOCATION_SIZE (10*1024*1024) /* 10MB pagefault free buffer */
#define MY_STACK_SIZE       (100*1024)      /* 100 kB dodatak za stek */
#define MY_PRINTF(dozvoljeno, ...)({if(dozvoljeno) {printf(__VA_ARGS__); fflush(stdout);}})/* Makro kojim kontrolisemo omogucenost ispisa! */

/* Enumeracija prioriteta(1-99)! */
enum PRIORITET {NAJMANJI_PRIORITET = 1,
				SREDNJI_PRIORITET = 5,
				NAJVECI_PRIORITET = 10};

// Izrazeno u milisekundama!(Jer se radi o enumu, koji zahtjeva cjelobrojne vrijednosti, kasnije se to konvertuje u s+ms)
enum SPAVANJE {	SPAVANJE_NMP = 300,  	  // Spavanje niti najmanjeg prioriteta!
			    SPAVANJE_SP  = 450,	  // Srednjeg prioriteta!
				SPAVANJE_NVP = 400};  // Najviseg prioriteta!

/* Struktura programske niti! */
typedef struct{
	float sekunde_spavanja;				// Broj sekundi, koje ce nit spavati!
	int prioritet_niti;					// Prioritet niti!
	int dodatna_velicina_steka;			// Velicina steka niti!
	bool ispis;							// Ispis stringa na ekran!
	int uvecanje_djeljenog_resursa;		// Cijeli broj koji definise uvecanje djeljenog resursa pr. niti!
	void* (*my_rt_thread)(void *args);	// Pokazivac na funkciju programske niti! 
}PARAMETRI;

/* Djeljeni resurs! */
static int shared_val = 0;

/* Mutex 'mtx' + mutex atr. */
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutexattr_t mtx_attr;

/* Pretvaranje ms u s! */
float ms_u_s(int ms){
	float s;

	int sec_i = ms / 1000;
	float sec_f = ms % 1000; 
	s = (float)sec_i + sec_f/1000;
	return s;
}

/* Funkcija za postavljanje prioriteta programske niti! */   
static void setprio(int prio, int sched){
	struct sched_param param;

   	// podesavanje prioriteta i schedulera
   	param.sched_priority = prio;
   	if (sched_setscheduler(0, sched, &param) < 0)
   		perror("sched_setscheduler");
}

/* Smjestanje steka programske niti u RAM! */
static void prove_thread_stack_use_is_safe(int stacksize, int do_log){
   	volatile char buffer[stacksize];
   	int i;
   
   	for (i = 0; i < stacksize; i += sysconf(_SC_PAGESIZE)) {
   		/* "Touch" za cijeli stek od programske niti */
   		buffer[i] = i;
   	}
   }
 
/* Funkcija za ispis greske, i izlazak! */  
static void error(int at){
   	/* Ispisi gresku i izadji */
   	fprintf(stderr, "Some error occured at %d\n", at);
   	exit(1);
}

/* Funkcija programske niti sa promjenjivim parametrima, i djeljenim resursom! */
static void *resource_thread_fn(void *args){
	int ret;
	PARAMETRI parametri = *(PARAMETRI*)args;
	bool ispis_dozvoljen = parametri.ispis;

	// Vremenska organizacija: (Pretvaranje sekundi u oblik strukture timespec!)
   	struct timespec ts;
	ts.tv_sec = (int)parametri.sekunde_spavanja; //Cijeli dio!
   	ts.tv_nsec = ((parametri.sekunde_spavanja - ts.tv_sec)*1000)*1000000; // Pretvaranje ms u ns!

   	setprio(parametri.prioritet_niti, SCHED_RR);  								// Postavljanje prioriteta niti!
   	prove_thread_stack_use_is_safe(parametri.dodatna_velicina_steka, 1); 		// 'Guranje' u RAM!
 
  	/* Spavanje ts sekundi, prilikom pokretanja, cime se postize zeljeni redoslijed izvrsavanja! */
   	clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
  
	// Mutex, jer imamo rad sa djeljenim objektom!
   	MY_PRINTF(ispis_dozvoljen, "Nit sa prioritetom %d, ceka da udje u MUTEX... \n",parametri.prioritet_niti);
	
	while((ret = pthread_mutex_trylock(&mtx))!=0){ // Mutex lock!

		if(ret == EBUSY) 	   printf("Mutex zauzet, EBUSY!\n");
		else if(ret == EINVAL) printf("Mutex zauzet, EINVAL!\n");
		else if(ret == EAGAIN) printf("Mutex zauzet, EAGAIN!\n");
		sleep(1);  //Provjera zauzetosti mutexa, periodicno!
	}
   	MY_PRINTF(ispis_dozvoljen, "Nit sa prioritetom %d, usla u MUTEX! \n",parametri.prioritet_niti);
	// Uvecanje djeljenog resursa!
	shared_val+=parametri.uvecanje_djeljenog_resursa;
    MY_PRINTF(ispis_dozvoljen, "Nit sa prioritetom %d, shared value %d\n",parametri.prioritet_niti, shared_val);

  	/* Spavanje ts sekundi prilikom izvrsavanja! */
	// Ako se radi o niti najmanjeg prioriteta, kako bismo djelimicno ublazili potencijalno dobro nepostavljena vremena spavanja, spava prilikom izvrsavanja za vrijednost koja je jednaka zbiru vremena druge 2 niti. Na taj nacin, ona ce sigurno sacekati dok ne dodje do pokretanja barem jedne od njih, odnosno nece se izvrsiti prije nego je jedna od ove 2 zapocela svoje izvrsavanje!
	float s = ms_u_s(((parametri.prioritet_niti==NAJMANJI_PRIORITET)?SPAVANJE_SP:0)+SPAVANJE_NVP);
	ts.tv_sec = (int)s; //Cijeli dio!
   	ts.tv_nsec = ((s - ts.tv_sec)*1000)*1000000; // Pretvaranje ms u ns!
   	clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);

	if((ret = pthread_mutex_unlock(&mtx))!=0)	 // Mutex unlock!
		error(9);

   	MY_PRINTF(ispis_dozvoljen, "Nit sa prioritetom %d, izasla iz MUTEX-a! \n",parametri.prioritet_niti);
   	return NULL;
}


/* Funkcija programske niti sa promjenjivim parametrima, i BEZ djeljenih resursa! */
static void *non_res_thread_fn(void *args){
	PARAMETRI parametri = *(PARAMETRI*)args;
	bool ispis_dozvoljen = parametri.ispis;

	// Vremenska organizacija:
   	struct timespec ts;
	ts.tv_sec = (int)parametri.sekunde_spavanja; //Cijeli dio!
   	ts.tv_nsec = ((parametri.sekunde_spavanja - ts.tv_sec)*1000)*1000000; // Pretvaranje ms u ns!

   	/* Spavanje ts sekundi! */
   	clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);  

	setprio(parametri.prioritet_niti, SCHED_RR);  	// Postavljanje prioriteta niti!
   	prove_thread_stack_use_is_safe(parametri.dodatna_velicina_steka, 1); 		// 'Guranje' u RAM!

   	MY_PRINTF(ispis_dozvoljen, "Nit sa prioritetom %d, zapocela izvrsavanje !\n",parametri.prioritet_niti);
	// Spavanje, moze se desiti da sa ovako podesenim vremenima, usljed spavanja niti, scheduler prepusti izvrsavanje nekoj drugoj niti, koja ce ispisivati nesto svoje, ili ce ostati na ovoj niti, i cekati!
   	clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);   
   	MY_PRINTF(ispis_dozvoljen, "Nit sa prioritetom %d, okoncala izvrsavanje !\n",parametri.prioritet_niti); 

	return NULL;
}
  
   
/* Kreiranje programskih niti! */
static pthread_t start_rt_thread(PARAMETRI* parametri){
   	pthread_t thread;
   	pthread_attr_t attr;
   
   	/* inicijalizacija programske niti */
   	if (pthread_attr_init(&attr))
   		error(1);

/**/if(pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED))
			error(11);

   	/* inicijalizacija memorije potrebne za stek */
   	if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + parametri->dodatna_velicina_steka))
   		error(2);


   	/* kreiranje programske niti */
   	pthread_create(&thread, &attr, parametri->my_rt_thread, (void*)parametri);

   	return thread;
} 
   
/* Konfiguracija alociranja memorije */
static void configure_malloc_behavior(void){
   	if (mlockall(MCL_CURRENT | MCL_FUTURE))
   		perror("mlockall failed:");
   
   	/* sbrk nema nazad */
   	mallopt(M_TRIM_THRESHOLD, -1);
   
   	/* iskljuci mmap */
   	mallopt(M_MMAP_MAX, 0);
}

/* Rezervisanje memorije, guranje 'svega' u RAM! */   
static void reserve_process_memory(int size){
   	int i;
   	char *buffer;
   	buffer = malloc(size);
   
   	for (i = 0; i < size; i += sysconf(_SC_PAGESIZE))
   		buffer[i] = 0;

   	free(buffer);
}
   
int main(int argc, char *argv[]){   
	int ret, mutex_protocol;

	if ((ret = pthread_mutexattr_init(&mtx_attr))!=0) // Inicijalizacija mutex atributa!
   		error(3);

	if ((ret = pthread_mutexattr_setprotocol(&mtx_attr, PTHREAD_PRIO_PROTECT))!=0)	//Postavljanje protokola na PRIO_PROTECT!
		error(4);

	if ((ret = pthread_mutexattr_getprotocol(&mtx_attr, &mutex_protocol)) != 0)	//Provjera postavljenog protokola!
		error(5);

	// Provjera da li je postavljeni protokol PRIO_PROTECT!
 	if (mutex_protocol != PTHREAD_PRIO_PROTECT)
   	 	error(6);
	else 
		printf("Protokol postavljen na PRIO_PROTECT!\n");

	if ((ret = pthread_mutex_init(&mtx, &mtx_attr))!=0)	// Inicijalizacija mutex objekta!
		error(7);

	configure_malloc_behavior();
	reserve_process_memory(PRE_ALLOCATION_SIZE);

	/* Definisanje parametara niti! */
	// Broj sekundi, Prioritet niti, Velicina steka, Ispis, Uvecanje djeljenog res., Funkcija programske niti!
	PARAMETRI par_thread1 = {ms_u_s(SPAVANJE_NVP),NAJVECI_PRIORITET,MY_STACK_SIZE,true,5,resource_thread_fn};// Nit najviseg prioriteta!
	PARAMETRI par_thread2 = {ms_u_s(SPAVANJE_SP),SREDNJI_PRIORITET,MY_STACK_SIZE,true,0,non_res_thread_fn};// Nit srednjeg prioriteta!
	PARAMETRI par_thread3 = {ms_u_s(SPAVANJE_NMP),NAJMANJI_PRIORITET,MY_STACK_SIZE,true,3,resource_thread_fn};// Nit najmanjeg prioriteta!

	/* Kreiranje programskih niti! */
	pthread_t thread1 = start_rt_thread(&par_thread1);
   	pthread_t thread2 = start_rt_thread(&par_thread2);
	pthread_t thread3 = start_rt_thread(&par_thread3);

	// Join nad prethodno kreiranim nitima!
	pthread_join(thread1,NULL);
   	pthread_join(thread2,NULL);
	pthread_join(thread3,NULL);

	printf("Press <ENTER> to exit\n");
	getchar();
	pthread_mutexattr_destroy(&mtx_attr); 	// prije unistavanja mutexa!
	pthread_mutex_destroy(&mtx);		  	// Unistavanje mutex objekta!
   	return 0;
   }
   
