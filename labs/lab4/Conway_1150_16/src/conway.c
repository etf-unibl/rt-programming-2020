/**
* Dejan Milojica(1150/16)
* Conway Game of Life
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#define DIM 10
#define VRIJEME_SPAVANJA_SISTEMA 1000 //Izrazeno u ms!
#define MAPA "mapa.txt"

/* Struktura, koja opisuje nit koja se izvrsava kao polje matrice! */
typedef struct{
	int x_pozicija;
	int y_pozicija;
	int prioritet;
	bool pobudjena;
}POLJE_MATRICE;

/* Staticke promjenjive! */
static sem_t semafor[DIM][DIM];
static sem_t main_semafor;
static pthread_mutex_t cs_mutex;
static bool KRAJ = false; // Promjenjiva kojom definisemo izlazak iz simulacije!

/* Matrica starih vrijednosti, azuriranih vrijednosti, kao i matrica polje, odnosno elemenata POLJE_MATRICE! */
static bool matrica_stara[DIM][DIM];
static bool matrica_nova[DIM][DIM];
static POLJE_MATRICE polje[DIM][DIM];

/* Deklaracija f-ja! */
void kopiranje_matrica();
void prikaz(void *u);
int kreiranje_semafora();
void unistavanje_semafora();
int kreiranje_polja_matrice(pthread_t elementi_matrice[DIM][DIM]);
void dodjeljivanje_prioriteta();
void* funkcija_polja_matrice(void* param);
int citanje_mape();
void* funkcija_prekida(void* param);

int main()
{
	pthread_t elementi_matrice[DIM][DIM];
	pthread_t nit_prekida;

	/* Citanje pocetnog stanja mape(matrice)! */
	if(citanje_mape()){
		printf("Neuspjesno citanje pocetnog stanja! Matrica ne postoji, ili nije na korektnom mjestu!\n");	
		return -1;	
	}
	sleep(1); //Spavanje sistema, kako bismo "primjetili" pocetno stanje matrice!

	if(pthread_mutex_init(&cs_mutex, NULL)){
		printf("Neuspjesno kreiranje mutex objekta!\n");	
		return -1;
	}

	if(kreiranje_semafora()){
		printf("Neuspjesno kreiranje semafora!\n");
		pthread_mutex_destroy(&cs_mutex);
		return -1;
	}

	if(kreiranje_polja_matrice(elementi_matrice)){
		printf("Neuspjesno kreiranje polja matrice!\n");
		unistavanje_semafora();
		pthread_mutex_destroy(&cs_mutex);	
		return -1;
	}

	// Aktiviranje prve niti!
	if(sem_post(&semafor[0][0])){
		printf("Neuspjesno pokretanje!\n");
		return -1;	
	}

	// Kreiranje niti prekida simulacije!
	if(pthread_create(&nit_prekida, NULL, funkcija_prekida, NULL)){
		unistavanje_semafora();
		pthread_mutex_destroy(&cs_mutex);
		return -1;
	}
	else{}

	while (!KRAJ) {
		// Stopiranje main niti, dok ne dodje na red!
		sem_wait(&main_semafor);
		prikaz(matrica_nova);
		kopiranje_matrica();
		sem_post(&semafor[0][0]);
		sleep(VRIJEME_SPAVANJA_SISTEMA/1000);
	}

	unistavanje_semafora();
	pthread_mutex_destroy(&cs_mutex);
	return 0;
}

/* Citanje mape(pocetne matrice) iz txt fajla! */
int citanje_mape(){
	FILE *fp;
	fp = fopen(MAPA,"r");
	if(fp==NULL){
		return -1;	
	}else{
		for(int x = 0; x < DIM; x++){
			for(int y = 0; y < DIM; y++){
				int i = fgetc(fp);
				if((char)i=='\n')
					y--;
				else{
					if(i!=EOF)
						matrica_stara[x][y]=matrica_nova[x][y] = i==48 ? false : true;
					else
						matrica_stara[x][y]=matrica_nova[x][y] = 0;
				}
			}
		}
		prikaz(matrica_nova);
		fclose(fp);
	}
return 0;
}

/* Nakon izvrsenog ciklusa evolucije, stara matrica poprima vrijednosti nove matrice! */
void kopiranje_matrica(){
	for(int x = 0; x < DIM; x++)
		for(int y = 0; y < DIM; y++)
			matrica_stara[x][y]=matrica_nova[x][y];
}

/* Funkcija niti prekida simulacije!*/
void* funkcija_prekida(void* param){
	(void)param;
	char znak;
	do{
		znak=getchar();
		sleep(0.4);
	}while(znak!='q');
	KRAJ = true;
return NULL;
}


/* Funkcija niti pojedinacnog polja matrice! */
void* funkcija_polja_matrice(void* param){

	POLJE_MATRICE novo_polje = *(POLJE_MATRICE*)param;

	while(!KRAJ){
		// Sve niti prilikom kreiranja se stopiraju.
    	sem_wait(&semafor[novo_polje.x_pozicija][novo_polje.y_pozicija]);

		// Nakon pokretanja niti, vrsimo ispitivanja, obradu, te pobudjivanje ostalih niti, manjeg prioriteta:
		// Ako se radi o polju [0][0], njegovo je samo da pobudi niti prioriteta 2, posto "ona" ima prioritet 1! 
		// Ako se ne radi o prvoj niti, ostale niti trebaju da evoluiraju na adekvatan nacin, te onda pobude ostale niti manjeg prioriteta!

		pthread_mutex_lock(&cs_mutex);
	
		int broj_zivih_polja=0;
		bool celija = matrica_stara[novo_polje.x_pozicija][novo_polje.y_pozicija]; // Provjera u kakvom je stanju posmatrana celija!

		int x_minus_jedan = novo_polje.x_pozicija - 1;
		int x_plus_jedan = novo_polje.x_pozicija + 1;
		int y_minus_jedan = novo_polje.y_pozicija - 1;
		int y_plus_jedan = novo_polje.y_pozicija + 1;

		// Provjera broja zivih komsija kod polja viseg/nizeg prioriteta(CITAMO RAZLICITE MATRICE)!		
		if(x_minus_jedan>=0){
			broj_zivih_polja+=matrica_stara[x_minus_jedan][novo_polje.y_pozicija];	
			if(y_minus_jedan>=0)
				broj_zivih_polja+=matrica_stara[x_minus_jedan][y_minus_jedan];	
			if(y_plus_jedan<DIM)
				broj_zivih_polja+=matrica_stara[x_minus_jedan][y_plus_jedan];	
		}else{}

		if(y_minus_jedan>=0){
			broj_zivih_polja+=matrica_stara[novo_polje.x_pozicija][y_minus_jedan];
			if(x_plus_jedan<DIM)
				broj_zivih_polja+=matrica_stara[x_plus_jedan][y_minus_jedan]; //
		}else{}	

		if(x_plus_jedan<DIM){
			broj_zivih_polja+=matrica_stara[x_plus_jedan][novo_polje.y_pozicija];//
			if(y_plus_jedan<DIM)
				broj_zivih_polja+=matrica_stara[x_plus_jedan][y_plus_jedan];	//	
		}else{}

		if(y_plus_jedan<DIM){
				broj_zivih_polja+=matrica_stara[novo_polje.x_pozicija][y_plus_jedan];	//	
		}else{}
		

		// Pravila 'prezivljavanje': 
		if(celija){ //Ako je celija/polje ziva/o: 
			if(broj_zivih_polja<2)
				matrica_nova[novo_polje.x_pozicija][novo_polje.y_pozicija] = 0;
			else if(broj_zivih_polja==2 || broj_zivih_polja==3)
				matrica_nova[novo_polje.x_pozicija][novo_polje.y_pozicija] = 1;
			else if(broj_zivih_polja>3)
				matrica_nova[novo_polje.x_pozicija][novo_polje.y_pozicija] = 0;
			else{}
		}else{
			if(broj_zivih_polja==3)
				matrica_nova[novo_polje.x_pozicija][novo_polje.y_pozicija] = 1;
			else{}
		}

		// Posljednja nit, pobudjuje main nit!
		if(novo_polje.x_pozicija==DIM-1 && novo_polje.y_pozicija==DIM-1){
			// Sve pobudjenosti vratiti na false!
			for(int x = 0; x < DIM; x++)
				for(int y = 0; y < DIM; y++)
					polje[x][y].pobudjena = false;
			sem_post(&main_semafor); // Pobudjivanje MAIN niti!

		}else{
			int x = novo_polje.x_pozicija + 1;
			int y = novo_polje.y_pozicija - 2;	

		//	Pobudjivanje niti istog prioriteta ispod!
			if(x<=DIM-1 && y>=0 && !polje[x][y].pobudjena){
				x = novo_polje.x_pozicija + 1;
				y = novo_polje.y_pozicija - 2;	 
				sem_post(&semafor[x][y]);
				polje[x][y].pobudjena = true;		
			}else{ //Kada vise nema, idemo ispocetka, pocevsi od niti veceg prioriteta!
				for(int x = 0; x < DIM; x++)
					for(int y = 0; y < DIM; y++){
						if(polje[x][y].prioritet == novo_polje.prioritet+1){
							sem_post(&semafor[x][y]);
							polje[x][y].pobudjena = true;
							x = y = DIM;						
						}else{}					
					}
			}
		}
	
		pthread_mutex_unlock(&cs_mutex);
	}
return NULL;
}

int kreiranje_polja_matrice(pthread_t elementi_matrice[DIM][DIM]){
	/* Dodjeljivanje prioriteta: */
	dodjeljivanje_prioriteta();
		
	for(int x = 0; x < DIM; x++)
		for(int y = 0; y < DIM; y++){
			polje[x][y].x_pozicija = x; polje[x][y].y_pozicija=y;
			if(pthread_create(&elementi_matrice[x][y], NULL, funkcija_polja_matrice, (void*)&polje[x][y]))
				return -1;
			else{}
		}

	return 0;
}

/* Dodjeljivanje prioriteta svakom od polja matrice(NITI)! */
void dodjeljivanje_prioriteta(){
	int prvi = 1;
	for(int i=0;i<DIM;i++){
		for(int j=0;j<DIM;j++){
			polje[i][j].prioritet = prvi+j; 
		}
		prvi+=2;
	}
}

void unistavanje_semafora(){
	for(int x = 0; x < DIM; x++)
		for(int y = 0; y < DIM; y++)
			sem_destroy(&semafor[x][y]);

	sem_destroy(&main_semafor);
}

/* Kreiranje semafora: */
int kreiranje_semafora(){
	for(int x = 0; x < DIM; x++)
		for(int y = 0; y < DIM; y++)
			if(sem_init(&semafor[x][y],0,0))
				return -1;	

	//Kreiranje main semafora:
	if(sem_init(&main_semafor,0,0))
		return -1;		
	return 0;
}

/* Prikaz stanja na konzolu! */
void prikaz(void *u)
{
	system("clear");
	bool (*univ)[DIM] = u;
	printf("----------------------\n");
	printf(" Conway Game of Life! \n");
	printf("----------------------\n");
	fflush(stdout);
	//printf("\033[H");
	for (int y = 0; y < DIM; y++){
		for (int x = 0; x < DIM; x++)
			printf(univ[y][x] ? "\033[07m  \033[m" : "  ");
		printf("\033[E");
	}
	fflush(stdout);
	printf("================================\n");
	printf("IZLAZ: q + ENTER!\n");
	printf("================================\n");
}
 
 
