#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BR_CELIJA 100
#define baza 10

static sem_t semSignal[BR_CELIJA];
static sem_t semFinishSistemSignal;
static sem_t semFinishSignal;
static sem_t semEvoSignal[BR_CELIJA];

/* Niz celija koje cine matricu. */
char niz_celija[ 2 * BR_CELIJA];

/* Vrijeme spavanja izmedju evolucija. */
int sleep_time;

/* Funkcija koju pokrecu sve niti koje predstavljaju jednu celiju. */
void* function_pthread(void* param)
{
    int id_sem=*(int*)param;
	int i, br_x, br_o;
	int arr_susjeda[8];

	/* Izracunavanje pozicije susjeda ove celije. */
	arr_susjeda[0] = ( id_sem % baza != 0 ) 					 					? id_sem - 1 			: - 1;
	arr_susjeda[1] = ( id_sem % baza != 0 && id_sem / baza != 0 ) 					? id_sem - ( baza + 1 ) : - 1;
	arr_susjeda[2] = ( id_sem / baza != 0 ) 						 				? id_sem - baza 		: - 1;
	arr_susjeda[3] = ( id_sem % baza != (baza - 1) && id_sem / baza != 0 )  		? id_sem - ( baza - 1 ) : - 1;
	arr_susjeda[4] = ( id_sem % baza != (baza - 1) )								? id_sem + 1 			: - 1;
	arr_susjeda[5] = ( id_sem % baza != (baza - 1) && id_sem / baza != (baza - 1) ) ? id_sem + ( baza + 1 ) : - 1;
	arr_susjeda[6] = ( id_sem / baza != (baza - 1) ) 								? id_sem + baza			: - 1;
	arr_susjeda[7] = ( id_sem / baza != (baza - 1) && id_sem % baza != 0 )			? id_sem + ( baza - 1 ) : - 1;

	do
	{
		br_x = 0, br_o = 0;
		
		/* Provjera stanja celija viseg prioriteta i pribavljanje podataka potrebnih za evoluciju. */
		for( i = 0; i < 8; ++i )
		{
			if ( arr_susjeda[i] != -1 )
			{
				if ( i < 4  )
				{
					sem_wait(&semSignal[arr_susjeda[i]]);
					sem_post(&semSignal[arr_susjeda[i]]);
				}
				niz_celija[arr_susjeda[i]] == 'x' ? ++br_x : ++br_o;
			}
		}

		/* Proces evolucije. */
		if( br_o == 3 && niz_celija[id_sem] == 'x' )
			niz_celija[id_sem + BR_CELIJA] = 'o';
		else if( ( br_o == 2 || br_o == 3 ) && niz_celija[id_sem] == 'o' )
			niz_celija[id_sem + BR_CELIJA] = 'o' ;
		else
			niz_celija[id_sem + BR_CELIJA] = 'x';

		/* Signalizacija kraja evolucije ove celije. */
		sem_post(&semSignal[id_sem]);
	}
	while( sem_wait(&semEvoSignal[id_sem]) == 0 && sem_trywait(&semFinishSignal) != 0 ); /* Ceka signalizaciju nove evolucije ili kraj programa. */
	
	return NULL;
}

/* Funkcija sistemske niti za ispis matrice celija i pokretanje nove evolucije. */
void* function_sistem_pthread(void* param)
{
	int file_stat = *(int*)param;
	int i,br=0;
	
	/* Ceka signal za kraj programa ili ceka novu evoluciju. */
	while( sem_wait(&semSignal[BR_CELIJA-1]) == 0 && sem_trywait(&semFinishSistemSignal) != 0)
	{		
		/* Upis evoluirane matrice calija u statisticki fajl. */
		i = write(file_stat,niz_celija,200);
	
		/* Ispis evoluiranje matrice celija. */
		i = system("clear");
		for( i = 0; i < BR_CELIJA; ++i )
		{
			(( i + 1 ) % baza ) == 0 ? printf("%c \n",niz_celija[ i + BR_CELIJA] ) : printf("%c ",niz_celija[ i + BR_CELIJA] );
			if ( niz_celija[ i + BR_CELIJA] == 'o' ) br++;
		}
		if ( !br ) 
		{
			sem_post(&semFinishSistemSignal);
			printf("Pritisnite 'enter' za kraj!\n");
		}
		else br = 0;
		
		
		/* Citanje nove matrice calija iz statistickog fajla. */
		i = read(file_stat,niz_celija,200);
		
		/* Spavanje izmedju dvije evolucije. */
		sleep(sleep_time);
		
		/* Postavljanje semafora za signalizaciju nove evolucije. */
		for( i = 0; i < BR_CELIJA; ++i )
			sem_trywait(&semSignal[i]);
		for( i = 0; i < BR_CELIJA; ++i )
			sem_post(&semEvoSignal[i]);
	}
	
	/* Postavljanje semafora za signalizaciju kraja programa. */
	for( i = 0; i < BR_CELIJA; ++i )
		sem_post(&semFinishSignal);
	for( i = 0; i < BR_CELIJA; ++i )
		sem_post(&semEvoSignal[i]);
			
	return NULL;
}

int main(int argc, char *argv[])
{
	
	FILE* file_desc;
	int file_stat;
    int i,j,niz[100],uslov = 0;
	char simbol;
    pthread_t arr_pthread[BR_CELIJA];
    pthread_t sistem_pthread;

	/* Provjera argumenata. */
	if(argc != 3)
	{	
		printf("Greska, provjerite argumente!\nArgumenti treba da budu vrijeme u 's' i matrica pocetnih stanja!\n");
		return 0;
	}
	
	/* Vrijeme spavanja izmedju dvije evolucije proslijedjeno kroz argumente. */
	sleep_time = atoi(argv[1]);

	/* Ucitavanje pocetne matrice calija. */
    if ( ( file_desc = fopen(argv[2], "r") )!= NULL )
	{
		for( i = 0; i < BR_CELIJA; ++i )
		{
			niz[i] = i;
			simbol = fgetc(file_desc);
			if ( simbol == 'x' || simbol == 'o' )
			{
				niz_celija[i] = simbol;
				niz_celija[ i + BR_CELIJA ] = simbol;
			}
			else
			{
				printf("Greska, sadrzaj datoteke "" %s "" nije ispravan!\n",argv[2]);
				fclose(file_desc);
				return 0;
			}
		}
		fclose(file_desc);
	}
	else 
	{
		printf("Greska, neuspjesno otvaranje datoteke "" %s ""!\n",argv[2]);
		return 0;
	}
	
	if( ( file_stat = open("/proc/stat_proc",O_RDWR) ) >= 0)
	{	
		/* Ispis pocetne matrice calija. */
		i = system("clear");
		
		/* Upis pocetne matrice calija u statisticki fajl. */
		i = write(file_stat,niz_celija,200);
		
		for( i = 0; i < BR_CELIJA; ++i )
		{
			(( i + 1 ) % baza ) == 0 ? printf("%c \n",niz_celija[i]) : printf("%c ",niz_celija[i]);
		}
		
		/* Citanje pocetne matrice calija u statisticki fajl. */
		i = read(file_stat,niz_celija,200);
		
		sleep(sleep_time);
		
		/* Formiranje semafora za kraj niti svake celije i za kraj sistemske niti. */
		if ( sem_init(&semFinishSignal, 0, 0) != 0 )
		{
			printf("Greska, semafor ""semFinishSignal"" nije formiran!\n");
			goto kraj;
		}
		if ( sem_init(&semFinishSistemSignal, 0, 0) != 0) 
		{
			printf("Greska, semafor ""semFinishSistemSignal"" nije formiran!\n");
			sem_destroy(&semFinishSignal);
			goto kraj;
		}
		
		/* Formiranje 100 semafora, signaliziraju koja celija je izvrsila evoluciju. */
		for( i = 0; i < BR_CELIJA; ++i )
		{
			if ( sem_init(&semSignal[i], 0, 0) != 0 )
			{
				printf("Greska, semafor ""semSignal[%d]"" nije formiran!\n",i);
				goto kraj_sem;
			}
			else if ( sem_init(&semEvoSignal[i], 0, 0) != 0 )
			{
				printf("Greska, semafor ""semEvoSignal[%d]"" nije formiran!\n",i);
				sem_destroy(&semSignal[i]);
				goto kraj_sem;
			}
		}
		
		/* Formiranje sistemske niti. */
		if ( pthread_create(&sistem_pthread,NULL,function_sistem_pthread,(void*)(&file_stat)) != 0 )
		{
			printf("Greska, sistemska nit nije formiran!\n");
			goto kraj_sem;
		}
		
		/* Formiranje 100 programskih niti, svaka nit predstavlja jednu celiju. */
		for( i = 0; i < BR_CELIJA; ++i )
			if( pthread_create(&arr_pthread[i],NULL,function_pthread,(void*)(&niz[i])) != 0 )
			{
				printf("Greska, %d celijska nit nije formiran!\n",i);
				for( j = 0; j < i; ++j )
				{
					pthread_cancel(arr_pthread[j]);
				}
				pthread_cancel(sistem_pthread);
				break;
			}

		/* Ceka karakter za kraj programa. */
		if( i == BR_CELIJA )
		{
			getchar();
			sem_post(&semFinishSistemSignal);
		}
		
		/* Cekanje na zavrsetak sistemske niti. */
		pthread_join(sistem_pthread, NULL);
		
		/* Cekanje na zavrsetak niti. */
		for( j = 0; j < i; ++j )
			pthread_join(arr_pthread[j], NULL);

		i = BR_CELIJA;
		goto kraj_sem;
	}
	else
	{
		printf("Greska! Statisticka datoteka ""/proc/stat_proc "" nije uspjesno otvorena.\n");
		return 0;
	}

kraj_sem:

	for( j = 0; j < i; ++j )
	{
		sem_destroy(&semSignal[j]);
		sem_destroy(&semEvoSignal[j]);
	}
	sem_destroy(&semFinishSignal);
	sem_destroy(&semFinishSistemSignal);
	goto kraj;

kraj:

	close(file_stat);
	return 0;
}
