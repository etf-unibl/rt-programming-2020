#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BR_CELIJA 100
#define baza 10

static sem_t semSignal[BR_CELIJA];
static sem_t semFinishSistemSignal;
static sem_t semFinishSignal;
static sem_t semEvoSignal[BR_CELIJA];

/* Struktura celije u matricu. */
typedef struct celija
{
	char trenutno_stanje;
	char evo_stanje;
}CELIJA;

/* Niz celija koje cine matricu. */
CELIJA niz_celija[BR_CELIJA];

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
				niz_celija[arr_susjeda[i]].trenutno_stanje == 'x' ? ++br_x : ++br_o;
			}
		}

		/* Proces evolucije. */
		if( br_o == 3 && niz_celija[id_sem].trenutno_stanje == 'x' )
			niz_celija[id_sem].evo_stanje = 'o';
		else if( ( br_o == 2 || br_o == 3 ) && niz_celija[id_sem].trenutno_stanje == 'o' )
			niz_celija[id_sem].evo_stanje = 'o' ;
		else
			niz_celija[id_sem].evo_stanje = 'x';

		/* Signalizacija kraja evolucije ove celije. */
		sem_post(&semSignal[id_sem]);
	}
	while( sem_wait(&semEvoSignal[id_sem]) == 0 && sem_trywait(&semFinishSignal) != 0 ); /* Ceka signalizaciju nove evolucije ili kraj programa. */
	
	return NULL;
}

/* Funkcija sistemske niti za ispis matrice celija i pokretanje nove evolucije. */
void* function_sistem_pthread(void* param)
{
	int sleep_time = *(int*)param;
	int i;
	
	/* Ceka signal za kraj programa ili ceka novu evoluciju. */
	while( sem_wait(&semSignal[BR_CELIJA-1]) == 0 && sem_trywait(&semFinishSistemSignal) != 0)
	{		
		/* Brisanje terminala. */
		i = system("clear");
		
		/* Ispis evoluiranje matrice celija i promjena trenutnog stanja u evoluirano. */
		for( i = 0; i < BR_CELIJA; ++i )
		{
			(( i + 1 ) % baza ) == 0 ? printf("%c \n",niz_celija[i].evo_stanje) : printf("%c ",niz_celija[i].evo_stanje);
			niz_celija[i].trenutno_stanje = niz_celija[i].evo_stanje;
		}
		printf("\n");

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
    int i,niz[100],uslov = 0;
	int sleep_time;
    pthread_t arr_pthread[BR_CELIJA];
    pthread_t sistem_pthread;

	/* Provjera argumenata. */
	if(argc != 2)
		return printf("Greska, provjerite argumente!\nArgument treba da bude vrijeme u 's'!\n");
	
	/* Vrijeme spavanja izmedju dvije evolucije proslijedjeno kroz argumente. */
	sleep_time = atoi(argv[1]);

	/* Formiranje pocetne matrice calija. */
	for( i = 0; i < BR_CELIJA; ++i )
	{
		niz[i] = i;
		if ( ( i % 2 ) == 0 )
			niz_celija[i].trenutno_stanje = niz_celija[i].evo_stanje = 'o';
		else
			niz_celija[i].trenutno_stanje = niz_celija[i].evo_stanje = 'x';
	}

	/* Ispis pocetne matrice calija. */
	i = system("clear");
	for( i = 0; i < BR_CELIJA; ++i )
		(( i + 1 ) % baza ) == 0 ? printf("%c \n",niz_celija[i].trenutno_stanje) : printf("%c ",niz_celija[i].trenutno_stanje);
	printf("\n");
	sleep(sleep_time);
	
	/* Formiranje semafora za kraj niti celija i za kraj sistemske niti. */
	uslov += sem_init(&semFinishSignal, 0, 0);
	uslov += sem_init(&semFinishSistemSignal, 0, 0);

	/* Formiranje 100 semafora, signaliziraju koja celija je izvrsila evoluciju. */
	for( i = 0; i < BR_CELIJA; ++i )
	{
		uslov += sem_init(&semSignal[i], 0, 0);
		uslov += sem_init(&semEvoSignal[i], 0, 0);
	}
	
	/* Formiranje sistemske niti. */
	uslov += pthread_create(&sistem_pthread,NULL,function_sistem_pthread,(void*)(&sleep_time));

	/* Formiranje 100 programskih niti, svaka nit predstavlja jednu celiju. */
	for( i = 0; i < BR_CELIJA; ++i )
		uslov += pthread_create(&arr_pthread[i],NULL,function_pthread,(void*)(&niz[i]));

	/* Provjera da li su sve niti i svi semafori kreirani. */
	if( uslov != 0 )
		return printf("Greska! Semafor ili nit nisu dobro kreiranji.\n");
	
	/* Ceka karakter za kraj programa. */
	getchar();
	sem_post(&semFinishSistemSignal);
	
	/* Cekanje na zavrsetak sistemske niti. */
	pthread_join(sistem_pthread, NULL);
	
	/* Cekanje na zavrsetak niti. */
	for( i = 0; i < BR_CELIJA; ++i )
		pthread_join(arr_pthread[i], NULL);

	/* Oslobadjanje resursa. */
	for( i = 0; i < BR_CELIJA; ++i )
	{
		sem_destroy(&semSignal[i]);
		sem_destroy(&semEvoSignal[i]);
	}
	sem_destroy(&semFinishSignal);

	return 0;
}
