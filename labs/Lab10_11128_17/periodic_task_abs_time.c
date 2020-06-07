#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

/* periodi ( taskova, tajmera i offset tajmera ) dati u ms */
#define PER_TASK1 60
#define PER_TASK2 80 
#define PER_TIMER 20
#define OFFSET 50

#define NSEC_PER_SEC 1000000000ULL

/*semafori za taskove i kraj programa*/
static sem_t sem_task1;
static sem_t sem_task2;
static sem_t sem_end;

/* funkcija koja racuna sledeci istek roka */
static inline void timespec_add_us(struct timespec *t, uint64_t d)
{
    d *= 1000000;
    d += t->tv_nsec;
    while (d >= NSEC_PER_SEC) {
        d -= NSEC_PER_SEC;
		t->tv_sec += 1;
    }
    t->tv_nsec = d;
}

/* funkcija koja ceka do sledece aktivacije funkcije job_body*/
static void wait_next_activation(struct timespec *r,int period)
{
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, r, NULL);
    timespec_add_us(r, period);
}

int start_periodic_timer(struct timespec *r, uint64_t offs)
{
    clock_gettime(CLOCK_REALTIME, r);
    timespec_add_us(r, offs);

    return 0;
}

/* funkcija koja ponovo pokrece taskove nakon njihovog perioda, a ona se pokrece nako svakog perioda tajmera*/
static void job_body(void)
{
    static int br_job;

    if ( br_job % ( PER_TASK1 / PER_TIMER ) == 0 ) 
    {
		sem_post(&sem_task1);
	}
	
    if ( br_job % ( PER_TASK2 / PER_TIMER ) == 0 ) 
    {
		sem_post(&sem_task2);
    }
    
    br_job++;
}

/* funkcija za task1, period pokretanja 60ms */
void* function_pthread_task1(void* param)
{
	int i,j;
	
	/* radi ako je postavljen signal za task1 */
	while( sem_wait(&sem_task1) == 0 )
	{
		/* ispisuje ako nema signala za kraj */
		if( sem_trywait(&sem_end) != 0 )
		{
			for (i=0; i<4; i++) 
			{
				for (j=0; j<1000; j++);
				printf("1");
				fflush(stdout);
			}
		}
		else 
			break;
	}
 
	return NULL;
}

/* funkcija za task2, period pokretanja 80ms */
void* function_pthread_task2(void* param)
{
	int i,j;

	/*radi ako je postavljen signal za task2 */
	while( sem_wait(&sem_task2) == 0 )
	{
		/* ispisuje ako nema signala za kraj */
		if( sem_trywait(&sem_end) != 0 )
		{
			for (i=0; i<6; i++) 
			{
				for (j=0; j<10000; j++);
				printf("2");
				fflush(stdout);
			}
		}
		else 
			break;
	}
	
	return NULL;
}

/* funkcija sistemske niti, ceka dok se ne pritisne enter */
void* function_pthread_sistem(void* param)
{
	printf("\nZA KRAJ PRITISNITE ENTER\n\n");
	getchar();
	
	/* postavlja semafore za kraj funkcija function_pthread_task1, function_pthread_task2 i while petlje u funkciji main */
	sem_post(&sem_end);
	sem_post(&sem_end);
	sem_post(&sem_end);
	sem_post(&sem_task1);
	sem_post(&sem_task2);
	
	return NULL;
}

/* formira semafore za signalizaciju pokretanja taskova i kraja programa, vraca 1 ako je uspjesno formiranje ili 0 ako nije uspjesno */
int formiraj_semafore(void)
{
	if ( sem_init(&sem_task1, 0, 0) != 0 )
	{
		printf("Greska, semafor ""sem_task1"" nije formiran!\n");
		return 0;
	}
	if ( sem_init(&sem_task2, 0, 0) != 0 )
	{
		printf("Greska, semafor ""sem_task2"" nije formiran!\n");
		sem_destroy(&sem_task1);
		return 0;
	}
	if ( sem_init(&sem_end, 0, 0) != 0 )
	{
		printf("Greska, semafor ""sem_end"" nije formiran!\n");
		sem_destroy(&sem_task1);
		sem_destroy(&sem_task2);
		return 0;
	}
	
	return 1;
}

/* formira niti za task1, task2 i sistemske niti, vraca 1 ako je uspjesno formiranje ili 0 ako nije uspjesno */
int formiraj_niti(pthread_t* pthread_task1,pthread_t* pthread_task2,pthread_t* pthread_sistem )
{
	if ( pthread_create(pthread_task1,NULL,function_pthread_task1,NULL ) != 0 )
	{
		printf("Greska, nit ""pthread_task1"" nije formiran!\n");
		return 0;
	}
	if ( pthread_create(pthread_task2,NULL,function_pthread_task2,NULL ) != 0 )
	{
		printf("Greska, nit ""pthread_task2"" nije formiran!\n");
		pthread_cancel(*pthread_task1);
		return 0;
	}
	if ( pthread_create(pthread_sistem,NULL,function_pthread_sistem,NULL ) != 0 )
	{
		printf("Greska, nit ""pthread_sistem"" nije formiran!\n");
		pthread_cancel(*pthread_task1);
		pthread_cancel(*pthread_task2);
		return 0;
	}
	
	return 1;
}

int main(int argc, char *argv[])
{
    int res;
	struct timespec r;
	pthread_t pthread_task1;
	pthread_t pthread_task2;
	pthread_t pthread_sistem;
	
	/* formiranje semafora i niti */
	if( formiraj_semafore() == 0 )
	{ 
		return 0;
	}
	if ( formiraj_niti( &pthread_task1, &pthread_task2, &pthread_sistem ) == 0 )
	{
		goto end;
	}
	
	/* startovanje tajmera */
    res = start_periodic_timer(&r,OFFSET);
    if (res < 0) 
    {
        perror("Start Periodic Timer");

        return -1;
    }

	/* ponavlja se sve dok se ne dobije signal za kraj */
    while( sem_trywait(&sem_end) != 0 ) 
    {
        wait_next_activation(&r,PER_TIMER);
        job_body();
    }
    
    /* cekanje zavrsetka pokrenutih niti */
    pthread_join(pthread_task1, NULL);
	pthread_join(pthread_task2, NULL);
	pthread_join(pthread_sistem, NULL);

	goto end;
	
end:	
	/* unistavanje semafora */
	sem_destroy(&sem_task1);
	sem_destroy(&sem_task2);
	sem_destroy(&sem_end);
	
    return 0;
}
