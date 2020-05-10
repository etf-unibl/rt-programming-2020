#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

//struktura celije: staro i novo stanje, prioritet, broj susjednoh celija sa vecim prioritetom
struct cell{
	int s_stanje,n_stanje;
	int priority;
	int count;
};

//staticke promenljive
static struct cell matrix[10][10];
static sem_t semafori[10][10];
static sem_t glavni_semafor;
static pthread_mutex_t mutex;

//deklaracija funkcija
void call_thread(void* pParam);
int evolve(int i_index,int j_index);
int check_number_of_priority_neighbours(int i_index,int j_index);
int check_number_of_live_neighbours(int i_index,int j_index);
int create_matrix_random();
int create_matrix_from_file();
void print_matrix();
void new_ciklus();
int init_semafora();
int destroy_semafora();
void create_threads(pthread_t threads[10][10]);
void shared_variable_access(int i_index,int j_index);

/*
  Funkcija koja predstavlja thread celija matrice matrix. Kao argumente prima indekse celije kao 
  dvocifrene projeve (npr 45 je isto kao [4][5])
*/
void* cell_thread(void* pParam){
	int index = *(int*)pParam;
	if(index < 0 || index > 99){
		printf("Greska prilikom kreiranja threada. Pogresan indeks prosljedjen:%d\n",index);
		return;
	}
	int j_index = index % 10, i_index = index / 10;
	int pom;
	while(1){
		sem_wait(&semafori[i_index][j_index]);
		evolve(i_index,j_index);
		if(i_index == 9 && j_index == 9){
			sem_post(&glavni_semafor);
			continue;
		}
		if(j_index + 1 <= 9)
			shared_variable_access(i_index, j_index + 1);
		if(i_index + 1 <= 9 && j_index - 1 >= 0)
			shared_variable_access(i_index + 1, j_index - 1);
		if(i_index + 1 <= 9)
			shared_variable_access(i_index + 1, j_index);
		if(i_index + 1 <= 9 && j_index + 1 <= 9)
			shared_variable_access(i_index + 1, j_index + 1);
	}
}

int main(int argc, char**argv){
	//Provjera validnosti broja argumenata
	if(argc != 3){
		printf("Pogresno koriscenje programa GameOfLife! Kao argument mu trebate poslat vreme spavanja (u ms) glavnog sistema i kao drugi argument mu saljete 0 za citanje iz fajla pocetno_stanje.txt ili 1 za random pocetno stanje.\n");
		return -1;
	}
	//provjera drugog argumenta i izbor da li se cita iz fila ili ce pocetno stanje biti radnom generated
	char input = *(char*)argv[2];
	if(input != '0' && input != '1'){
		printf("Greska. Drugi argument samo moze biti 0 ili 1.\n");
		return -1;
	}else{
		if(input == '0')
			create_matrix_from_file();
		else if(input == '1')
			create_matrix_random();
	}
	//Inicjalizacija mutexa, semfaora i kreiranje tredova
	pthread_t threads[10][10];
	int pom;
	pom = pthread_mutex_init(&mutex,NULL);
	if(pom != 0){
		printf("Greska prilikom inicjalizacije mutexa!\n");
		pthread_mutex_destroy(&mutex);
		return -1;
	}
	pom = init_semafora();
	if(pom != 20){
		printf("Greska prilikom inicjalizacije semafora!\n");
		destroy_semafora();
		pthread_mutex_destroy(&mutex);
		return -1;
	}
	create_threads(threads);
	char *p;
	long sleep_time = strtol(argv[1],&p,10);
	//ispis pocetne vrednosti matrice matrix i pokretanje programa
	print_matrix();
	sleep(sleep_time);
	sem_post(&semafori[0][0]);
	
	while(1){
		sem_wait(&glavni_semafor);
		new_ciklus();
		print_matrix();
		sleep(sleep_time);
		sem_post(&semafori[0][0]);
	}
	//unistavanje mutexa i semafora
	pthread_mutex_destroy(&mutex);
	destroy_semafora();
	return 0;
}


/*
  Funckija prolazi kroz sva pravila od Game of Life za datu celiju i zadaje odgovarajuce n_stanje.
  Kao argumente prima indekse celije u matrici matrix. Funkcija vraca 0 ukoliko je uspjesno odredila 
  novo stanje ili -1 ako je poslati indeksi van granica matrice matrix.
*/
int evolve(int i_index,int j_index){
	bool s_stanje;
	int pom;
	if(i_index >= 0 && i_index <= 9 && j_index >= 0 && j_index <= 9){
		s_stanje = matrix[i_index][j_index].s_stanje;
		pom = check_number_of_live_neighbours(i_index,j_index);
		if(s_stanje == 0 && pom == 3){
			matrix[i_index][j_index].n_stanje = 1;
		}else if(s_stanje == 0 && pom != 3){
			matrix[i_index][j_index].n_stanje = 0;
		}else if(s_stanje == 1 && pom < 2){
			matrix[i_index][j_index].n_stanje = 0;
		}else if(s_stanje == 1 && pom > 3){
			matrix[i_index][j_index].n_stanje = 0;
		}else{
			matrix[i_index][j_index].n_stanje = 1;
		}
		return 0;
	}
	printf("Celija (%d,%d) ne postoji!!!\n", i_index, j_index);
	fflush(stdout);
	return -1;
}

/*
  Funkcija vraca broj susjednih celija koje imaju veci prioritet. Kao argumente prima indekse celije
  u matrici matrix.
*/
int check_number_of_priority_neighbours(int i_index,int j_index){
	int priority_neighbours_count = 0;
	if((i_index - 1) >= 0 && (j_index - 1) >= 0)
		priority_neighbours_count++;
	if((i_index - 1) >= 0)
		priority_neighbours_count++;	
	if((i_index - 1) >= 0 && (j_index + 1) <= 9)
		priority_neighbours_count++;
	if((j_index - 1) >= 0)
		priority_neighbours_count++;
	return priority_neighbours_count;
}

/*
  Funkcija vraca broj zivih susjednih celija. Kao argumente prima indekse celije u matrici matrix.
  Prije svake provjere zivosti neke od susjednih celija provjeravaju se da li celija uopste ima 
  susjeda na tom mjestu tj. da li je celija na ivicama matrice. Ukoliko nema odgovarajuceg susjeda
  preskace njegovu provjeru zivosti.
*/
int check_number_of_live_neighbours(int i_index,int j_index){
	int live_neighbours_count = 0;
	if((i_index - 1) >= 0 && (j_index - 1) >= 0)
		if(matrix[i_index - 1][j_index - 1].s_stanje == 1)
			live_neighbours_count++;
	if((i_index - 1) >= 0)
		if(matrix[i_index - 1][j_index].s_stanje == 1)
			live_neighbours_count++;	
	if((i_index - 1) >= 0 && (j_index + 1) <= 9)
		if(matrix[i_index - 1][j_index + 1].s_stanje == 1)
			live_neighbours_count++;
	if((j_index - 1) >= 0)
		if(matrix[i_index][j_index - 1].s_stanje == 1)
			live_neighbours_count++;
	if((j_index + 1) <= 9)
		if(matrix[i_index][j_index + 1].s_stanje == 1)
			live_neighbours_count++;
	if((i_index + 1) <= 9 && (j_index - 1) >= 0)
		if(matrix[i_index + 1][j_index - 1].s_stanje == 1)
			live_neighbours_count++;
	if((i_index + 1) <= 9)
		if(matrix[i_index + 1][j_index].s_stanje == 1)
			live_neighbours_count++;	
	if((i_index + 1) <= 9 && (j_index + 1) <= 9)
		if(matrix[i_index + 1][j_index + 1].s_stanje == 1)
			live_neighbours_count++;
	return live_neighbours_count;
}

/*
  Funkcija popunjava matricu tako sto u stara stanja nasumicno odabranih celija stavlja 1 ,sta oznacava 
  da je celija ziva. A u nova stanja svih celija stavlja 0 jer su trenutno nepoznata. Takodje daje celiji 
  odgovarajuci prioritet i broj susjeda sa vecim prioritetom.Vraca zbir indeksa zadnje upisane vrijednosti 
  u matricu.Za matricu 10*10 vratice 20(cisto radi provjere).
*/
int create_matrix_random(){
	int i,j;
	int rng;
	int priority = 0;
	for(i = 0;i < 10; i++){
		for(j = 0;j < 10; j++){
			rng = rand() % 2;
			if(rng == 1){
				matrix[i][j].s_stanje = 1;
			}else{
				matrix[i][j].s_stanje = 0;
			}
			matrix[i][j].n_stanje = 0;
			matrix[i][j].priority = priority + j;
			matrix[i][j].count = check_number_of_priority_neighbours(i,j);
		}
		priority += 2;
	}
	return i + j;
}

/*
  Funkcija popunjava matricu matrix tako sto u stara stanja upisuje procitanje vrednosti iz fajla 
  pocetno_stanje.txt a u nova stanja upisuje 0 jer je novo stanje trenutno nepoznato. Takodje daje celiji 
  odgovarajuci prioritet i broj susjeda sa vecim prioritetom.Vraca zbir indeksa zadnje upisane vrijednosti 
  u matricu ako je uspjesno obavljeno kreiranje matrice a -1 ako je nastao problem sa citanjem datoteke.
*/
int create_matrix_from_file(){
	int i = 0, j = 0, priority = 0;
	int c, ucitani_svi=0;
	FILE* fp;
	if((fp = fopen("pocetno_stanje.txt","r")) == NULL){
		printf("Datoteka pocetno_stanje.txt neuspjesno otvorena!\n");
		return -1;
	}
	while(i < 10){
		while(j < 10){
			c = fgetc(fp);
			if(c == 10){
				j--;
				continue;
			}else{
				if(c != EOF){
					matrix[i][j].s_stanje = c == 49 ? 1 : 0;
					matrix[i][j].n_stanje = 0;
					matrix[i][j].priority = priority + j;
					matrix[i][j].count = check_number_of_priority_neighbours(i,j);
				}else{
					matrix[i][j].s_stanje = 0;
					matrix[i][j].n_stanje = 0;
					matrix[i][j].priority = priority + j;
					matrix[i][j].count = check_number_of_priority_neighbours(i,j);
				}
			}
			j++;
		}
		j = 0;
		i++;
		priority += 2;
	}
	fclose(fp);
	return i + j;	
}

/*
  Funkcija za ispis zaglavlja i stanja celija iz matrice matrix na ekran
*/
void print_matrix(){
	int i,j;
	int pom = system("clear");
	if(pom != 0){
		printf("Greska prilikom ciscenja ekrana!\n");
		return;
	}
	printf("================\n");
	printf("   Conway GoL   \n");
	printf("================\n");
	for(i = 0; i < 10; i++){
		printf("  |");
		for(j = 0; j < 10; j++){
			if(matrix[i][j].s_stanje == 1)
				printf("*");
			else
				printf(" ");
		}
		printf("|\n");
	}
	printf("================\n");
	return;
}

/*
  Funkcija koja podesi matricu matrix za novi ciklus. Staro stanje postane novo stanje a novo stanje postane
  nepoznato. Takodje broj susjednih celija sa vecim prioritetom se restartuje jer je mjenjano tokom ciklusa.
*/
void new_ciklus(){
	int i,j;
	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			matrix[i][j].s_stanje = matrix[i][j].n_stanje;
			matrix[i][j].n_stanje = 0;
			matrix[i][j].count = check_number_of_priority_neighbours(i,j);
		}
	}
}

/*
  Funkcija za inicjalizaciju semafora.Poziva se na pocetku programa.Vraca zbir indeksa zadnjeg incjalizovanog 
  semafora u matrici semafori.
*/
int init_semafora(){
	int i,j;
	sem_init(&glavni_semafor,0,0);
	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			sem_init(&semafori[i][j], 0, 0);
		}
	}
	return i + j;
}

/*
  Funkcija za unistavanje semafora. Poziva se na kraju programa. Vraca zbir indeksa zadnjeg unistenog
  semafora u matrici semafori.
*/
int destroy_semafora(){
	int i,j;
	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			sem_destroy(&semafori[i][j]);
		}
	}
	return i+j;
}

/*
  Funkcija za kreiranje svih threadova. Poziva se na pocetku programa.
  PS. Znam da nije optimalno koristit ovakvo rjesenje sa sleepovima ali pokusao sam i sa 100 razlicitih varijabli i opet nastaje problem pri 
	  kreiranju tredova, pa ukoliko nije problem da mi preporucete bolje rjesenje na odbrani zadatka.
*/
void create_threads(pthread_t threads[10][10]){
	int i,j,pom;
	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			pom = (i * 10) + j;
			usleep(10000);
			pthread_create(&threads[i][j],NULL,cell_thread,(void*)&pom);
			usleep(10000);
		}
	}
}
	
/*
  Funkcija koja se koristi za pristup djeljenom resursu count. Kao argumente prima indekse celije
  u matrici matrix kojoj se pristupa da bi se smanjio count. Kada count dodje do 0 ispunjeni su
  uslovi da thread koji prati tu celiju nastavi svoj poso.
*/
void shared_variable_access(int i_index,int j_index){
	pthread_mutex_lock(&mutex);
	if(matrix[i_index][j_index].count == 1){
		sem_post(&semafori[i_index][j_index]);
	}
	matrix[i_index][j_index].count--;
	pthread_mutex_unlock(&mutex);
}
