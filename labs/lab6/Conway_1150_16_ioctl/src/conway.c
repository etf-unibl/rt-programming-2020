/**
* Dejan Milojica(1150/16)
* Conway Game of Life
* Ukljucujuci Statisticke podatke o celijama, koji su smjesteni unutar /dev/fajl!
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "data.h"
 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

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
static bool KRAJ = false;

/* Matrica starih vrijednosti, azuriranih vrijednosti, kao i matrica polje, odnosno elemenata POLJE_MATRICE! */
static bool matrica_stara[DIM][DIM];
static bool matrica_nova[DIM][DIM];
static POLJE_MATRICE polje[DIM][DIM];
static STATISTIKA statisticki_podaci;
int file_desc;

/* Deklaracija f-ja! */
void kopiranje_matrica();
void prikaz(void *u);
int kreiranje_semafora();
void unistavanje_semafora();
int kreiranje_polja_matrice(pthread_t elementi_matrice[DIM][DIM]);
void dodjeljivanje_prioriteta();
void* funkcija_polja_matrice(void* param);
void* funkcija_prekida(void* param);
int citanje_mape();
void ispis_statistike(STATISTIKA statistika_nova);

int main()
{
    int ret_val;

	pthread_t elementi_matrice[DIM][DIM];
	pthread_t nit_prekida;

    /* Open /dev/etx_device device. */
    file_desc = open(dev_file, O_RDWR);  // Definisano unutar header-a data.h!

    if(file_desc < 0)
    {
        printf("'/dev/etx_device' file isn't open\n");
        printf("Try:\t1) Check does '/dev/etx_device' node exist\n\t2)'chmod 666 /dev/etx_device'\n\t3) ""insmod"" ioctl module\n");
        return -1;
    }

	//Upis NULTIH statistickih podataka, tako da ukoliko prilikom insertovanja modula jednom, pokrenemo i zaustavimo aplikaciju vise puta, svaki put se krece od "nule", odnosno, prilikom novog pokretanja, brisu se stari sacuvani podaci!
    ret_val = ioctl(file_desc, WR_VALUE, (int32_t*) &statisticki_podaci);
	if(ret_val){
		printf("Problem sa upisom statistike! Unesite q+ENTER, za IZLAZ!\n");
    	close(file_desc);	
		return -1;	
	}


	/* Citanje pocetnog stanja mape(matrice)! */
	if(citanje_mape()){
		printf("Neuspjesno citanje pocetnog stanja! Matrica ne postoji, ili nije na korektnom mjestu!\n");
    	close(file_desc);	
		return -1;	
	}
	sleep(1); //Spavanje sistema, kako bismo "primjetili" pocetno stanje matrice!

	if(pthread_mutex_init(&cs_mutex, NULL)){
		printf("Neuspjesno kreiranje mutex objekta!\n");
    	close(file_desc);		
		return -1;
	}

	if(kreiranje_semafora()){
		printf("Neuspjesno kreiranje semafora!\n");	
    	close(file_desc);	
		return -1;
	}

	if(kreiranje_polja_matrice(elementi_matrice)){
		printf("Neuspjesno kreiranje polja matrice!\n");
		unistavanje_semafora();
		pthread_mutex_destroy(&cs_mutex);	
    	close(file_desc);	
		return -1;
	}

	// Aktiviranje prve niti!
	if(sem_post(&semafor[0][0])){
		printf("Neuspjesno pokretanje!\n");
    	close(file_desc);	
		return -1;	
	}

	// Kreiranje niti prekida simulacije!
	if(pthread_create(&nit_prekida, NULL, funkcija_prekida, NULL)){
    	close(file_desc);		
		return -1;
	}

/* Statisticki podaci se upisuju po zavrsetku/pocetku jednog ciklusa! */
/* Citanje stat. podataka, se obavlja u funkciji "prikaz", koja prikazuje stanje matrice na ekran korisnika! */
	while (!KRAJ) { 		
		//Upis statistickih podataka!
        ret_val = ioctl(file_desc, WR_VALUE, (int32_t*) &statisticki_podaci);
		if(ret_val)
			printf("Problem sa upisom statistike! Unesite q+ENTER, za IZLAZ!\n");

		// Stopiranje main niti, dok ne dodje na red!
		sem_wait(&main_semafor);
		prikaz(matrica_nova);	// Prikaz trenutnog stanja, kao i ispis statistike!
		kopiranje_matrica();
		sem_post(&semafor[0][0]);
		sleep(VRIJEME_SPAVANJA_SISTEMA/1000);
		statisticki_podaci.broj_generacija++; //Broj iteracija se povecava!
		/* Ukoliko je potrebno posmatrati na nivou ciklusa, a ne simulacije, radimo resetovanje! */
		//statisticki_podaci.broj_rodjenih=0;statisticki_podaci.broj_umrlih=0;
	}

	unistavanje_semafora();
	pthread_mutex_destroy(&cs_mutex);
    /* Close file. */
    close(file_desc);
	return 0;
}

/* Ispis Statistike! */
void ispis_statistike(STATISTIKA statistika_nova){
    printf("Statistika: \n");
	printf("================================\n");
	printf("  Broj zivih celija:    %d\n",statistika_nova.broj_zivih_celija);
	printf("  Broj rodjenih celija: %d\n",statistika_nova.broj_rodjenih);
	printf("  Broj umrlih celija:   %d\n",statistika_nova.broj_umrlih);
	printf("  Broj generacija:      %d\n",statistika_nova.broj_generacija);
	printf("IZLAZ: q + ENTER!\n");
	printf("================================\n");
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
					if(i!=EOF){
						matrica_stara[x][y]=matrica_nova[x][y] = i==48 ? false : true;
					if(i!=48)
						statisticki_podaci.broj_zivih_celija++;
					}
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
	(void)param; //Unused parametar!
	char znak;
	do{
		znak=getchar();
		sleep(0.4); //250ms
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
		int x_plus_jedan  = novo_polje.x_pozicija + 1;
		int y_minus_jedan = novo_polje.y_pozicija - 1;
		int y_plus_jedan  = novo_polje.y_pozicija + 1;

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
			if(broj_zivih_polja<2){
				matrica_nova[novo_polje.x_pozicija][novo_polje.y_pozicija] = 0;
				statisticki_podaci.broj_zivih_celija--;
				statisticki_podaci.broj_umrlih++;
			}
			else if(broj_zivih_polja==2 || broj_zivih_polja==3)
				matrica_nova[novo_polje.x_pozicija][novo_polje.y_pozicija] = 1;
			else if(broj_zivih_polja>3){
				matrica_nova[novo_polje.x_pozicija][novo_polje.y_pozicija] = 0;
				statisticki_podaci.broj_zivih_celija--;
				statisticki_podaci.broj_umrlih++;
			}
			else{}
		}else{
			if(broj_zivih_polja==3){
				matrica_nova[novo_polje.x_pozicija][novo_polje.y_pozicija] = 1;
				statisticki_podaci.broj_zivih_celija++;
				statisticki_podaci.broj_rodjenih++;
			}
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

// Promjenjiva tipa STATISTIKA, u koju upisujemo procitanu statistiku.
	STATISTIKA statistika_nova;
	//Ucitavanje, i prikaz statistike!
    int ret_val = ioctl(file_desc, RD_VALUE, (int32_t*) &statistika_nova);
	if(ret_val)
		printf("Neispravno citanje statistike!\n");
	else
    	ispis_statistike(statistika_nova);
}


        
