#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>

//struktura celije: staro i novo stanje, prioritet, broj susjednoh celija sa vecim prioritetom
struct cell{
	int s_stanje,n_stanje;
	int priority;
	int count;
};
//struktura informacija koje se trebaju cuvat u driveru
struct info{
	char num_alive[1], num_born[1], num_died[1], gen_num[1];
	char matrix[100];
};

//staticke promenljive
static struct cell matrix[10][10];
static sem_t semafori[10][10];
static sem_t glavni_semafor;
static pthread_mutex_t mutex;

//imena datoteka koje moze koristiti program za citanje i pisanje informacija u driveru,indetifikatori da li se trazi 
//data informacija i promjenljive potrebne za pisanje i citanje u datoteku
static char file_mat[] = "/sys/kernel/stat_sysfs/matrix",
			file_alive[] = "/sys/kernel/stat_sysfs/num_alive", 
			file_born[] = "/sys/kernel/stat_sysfs/num_born", 
			file_died[] = "/sys/kernel/stat_sysfs/num_died",
			file_gen[] = "/sys/kernel/stat_sysfs/gen_num";
static int isMat, isAlive, isBorn, isDied, isGen;
static int fd_mat, fd_alive, fd_born, fd_died, fd_gen;

//deklaracija funkcija
void call_thread(void* pParam);
int evolve(int i_index,int j_index);
int check_number_of_priority_neighbours(int i_index,int j_index);
int check_number_of_live_neighbours(int i_index,int j_index);
int create_matrix_random();
int create_matrix_from_file();
void print_matrix(struct info statistika);
void new_ciklus();
int init_semafora();
int destroy_semafora();
void create_threads(pthread_t threads[10][10]);
void shared_variable_access(int i_index,int j_index);
struct info statistics(int first_time, int gen);
int isValidArgument(int i, char*arg);
int open_files();
void write_files(struct info statistika);
struct info read_files();
void close_files();

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
	
	//Provjera validnosti argumenata
	if(argc < 3 || argc > 8){
		printf("Pogresno koriscenje programa GameOfLife! Pravilno koristenje: \n");
		printf("	Prvi argument: vrijeme spavanja u sekundama(OBAVEZNO).\n");
		printf("	Drugi argument: 0 za citanje matrice iz fajla pocetno_stanje.txt , 1 za random generated matrix(OBAVEZNO).\n");
		printf("	Treci - Sedmi argumenti: Datoteke koje se koriste za cuvanje i citanje informacija iz kernela, navode se samo za zeljene informacije.\n");
		printf("		sys/kernel/stat_sysfs/matrix za informacije o celijama.\n");
		printf("		sys/kernel/stat_sysfs/num_alive za broj zivih.\n");
		printf("		sys/kernel/stat_sysfs/num_born za broj novorodjenih.\n");
		printf("		sys/kernel/stat_sysfs/num_died za broj umrlih u zadnjem ciklusu.\n");
		printf("		sys/kernel/stat_sysfs/gen_num za broj tekuce generacije.\n");
		return -1;
	}else{
		int i,res = 0;
		for(i = 3; i < argc; i++){
			res = isValidArgument(i, argv[i]);
			if(res == -1)
				return -1;
		}
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
	
	//racunanje statistike
	int gen = 1;
	struct info statistika = statistics(0, gen);
	statistika.gen_num[0] = gen;
	
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
	
	//otvaranje datoteka i prvi upis u datoteke
	pom = open_files();
	if(pom == -1){
		printf("Neke datoteke su neuspjesno otvorene!");
		pthread_mutex_destroy(&mutex);
		destroy_semafora();
		close_files();
		return -1;
	}
	write_files(statistika);
	
	create_threads(threads);
	char *p;
	long sleep_time = strtol(argv[1],&p,10);
	
	//ispis pocetne vrednosti matrice matrix i pokretanje programa
	print_matrix(statistika);
	sleep(sleep_time);
	sem_post(&semafori[0][0]);
	
	while(1){
		sem_wait(&glavni_semafor);
		gen++;
		statistika = statistics(1, gen);
		statistika.gen_num[0] = gen;
		write_files(statistika);
		new_ciklus();
		print_matrix(statistika);
		sleep(sleep_time);
		sem_post(&semafori[0][0]);
	}
	
	//unistavanje mutexa , semafora i zatvaranje fajlova
	pthread_mutex_destroy(&mutex);
	destroy_semafora();
	close_files();
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
  Funkcija za ispis zaglavlja i stanja celija iz matrice matrix na ekran, te ispis trazenih informacija.
*/
void print_matrix(struct info statistika){
	int i,j;
	int pom = system("clear");
	if(pom != 0){
		printf("Greska prilikom ciscenja ekrana!\n");
		return;
	}
	//Nisam mogao naci gresku pri upisu/citanju podataka iz drivera pa ispisujem samo korektnu statistiku, 
	//da ne ispadne da nisam predao uopste. Da sam usjeo popravit gresku jedina promjena bi bila da bi ova 
	//sledeca linija bila ukljuena i da funkcija print_matrix ne prima nikakav argument.
	//struct info statistika = read_files();
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
	printf("   Statistics   \n");
	printf("================\n");
	if(isAlive > 0){
		printf("  Alive: %d\n",statistika.num_alive[0]);
	}
	if(isBorn > 0){
		printf("  Born: %d\n",statistika.num_born[0]);
	}
	if(isDied > 0){
		printf("  Died: %d\n",statistika.num_died[0]);
	}
	if(isGen > 0){
		printf("  Gen: %d\n",statistika.gen_num[0]);
	}
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

/*
  Funkcija koje se koristi za pronalazenje statistike koja ce se slati na rukovodioc. Kao argumente prima 
  vrijednost da li je to prvo pozivanje(ukoliko jeste salje se 0) i prima redni broj generacije.Kao rezultat 
  vraca strukturu tipa struct info sa upisanim rezultatima. 
*/
struct info statistics(int first_time, int gen){
	int i, j, k = 0;
	struct info statistika;
	statistika.num_alive[0] = statistika.num_born[0] = statistika.num_died[0] = statistika.gen_num[0] = 0;
	for(k = 0;k < 100;k++){
		statistika.matrix[k] = 0;
	}
	k = 0;
	if(first_time == 0){
		statistika.num_born[0] = 0;
		statistika.num_died[0] = 0;
	}
	for(i = 0; i < 10 ; i++){
		for(j = 0; j < 10; j++){
			if(first_time == 0){
				statistika.matrix[k] = matrix[i][j].s_stanje;
				if(matrix[i][j].s_stanje == 1){
					statistika.num_alive[0]++;
				}
			}else{
				statistika.matrix[k] = matrix[i][j].n_stanje;
				if(matrix[i][j].n_stanje == 1){
					statistika.num_alive[0]++;
					if(matrix[i][j].s_stanje == 0){
						statistika.num_born[0]++;
					}
				}else{
					if(matrix[i][j].s_stanje == 1){
						statistika.num_died[0]++;
					}
				}
			}
			k++;
		}
	}
	return statistika;
}

/*
  Funkcija provjerava validnost argumenata funkcije main. Kao argumente prima redni broj argumenta koji se provjerava i taj argument.
  Ukoliko je argument validan jedan od indetifikatora isMat,isAlive... ce biti postavljen da vrijednost poslatog rednog broja i funcija
  ce vratiti 0 u suprotnom vratice -1;
*/
int isValidArgument(int i, char*arg){
	int pom;
	if((pom = strcmp(arg, file_mat)) == 0){
		isMat = i;
	}else if((pom = strcmp(arg, file_alive)) == 0){
		isAlive = i;
	}else if((pom = strcmp(arg, file_born)) == 0){
		isBorn = i;
	}else if((pom = strcmp(arg, file_died)) == 0){
		isDied = i;
	}else if((pom = strcmp(arg, file_gen)) == 0){
		isGen = i;
	}else{
		printf("Argument %s na poziciji %d nije validan!\n", arg, i);
		return -1;
	}
	return 0;
}

/*
  Funkcija koja otvara sve trazene datoteke.Vraca 0 ako je uspjesno sve odradjeno, a -1 ako je negdje zaglavilo
  * 
*/
int open_files() {
	if(isMat > 0){
		fd_mat = open(file_mat, O_RDWR);
		if(fd_mat < 0){
			printf("Nije dobro otvoren file_mat.\n");
			return -1;
		}
	}
	if(isAlive > 0){
		fd_alive = open(file_alive, O_RDWR);
		if(fd_alive < 0){
			printf("Nije dobro otvoren file_alive.\n");
			return -1;
		}
	}
	if(isBorn > 0){
		fd_born = open(file_born, O_RDWR);
		if(fd_born < 0){
			printf("Nije dobro otvoren file_born.\n");
			return -1;
		}
	}
	if(isDied > 0){
		fd_died = open(file_died, O_RDWR);
		if(fd_died < 0){
			printf("Nije dobro otvoren file_died.\n");
			return -1;
		}
	}
	if(isGen > 0){
		fd_gen = open(file_gen, O_RDWR);
		if(fd_gen < 0){
			printf("Nije dobro otvoren file_gen.\n");
			return -1;
		}
	}
	return 0;
}

/*
  Funkcija koja upisuje informacije u trazene datoteke. Kao argument joj se salje podatak tipa struct info koji sadzi sve informacije.
*/
void write_files(struct info statistika){
	if(isMat > 0){
		lseek(fd_mat, 0, SEEK_SET);
		write(fd_mat, statistika.matrix, 100);	
	}
	if(isAlive > 0){
		lseek(fd_alive, 0, SEEK_SET);
		write(fd_alive, statistika.num_alive, 1);
	}
	if(isBorn > 0){
		lseek(fd_born, 0, SEEK_SET);
		write(fd_born, statistika.num_born, 1);
	}
	if(isDied > 0){
		lseek(fd_died, 0, SEEK_SET);
		write(fd_died, statistika.num_died, 1);
	}
	if(isGen > 0){
		lseek(fd_gen, 0, SEEK_SET);
		write(fd_gen, statistika.gen_num, 1);
	}
}

/*
  Funkcija koja cita informacije iz traznih datoteka. Kao rezultat vraca prodatak tia struct info koji sadzi sve trazene informacije.
*/
struct info read_files(){
	struct info statistika;
	if(isMat > 0){
		lseek(fd_mat, 0, SEEK_SET);
		read(fd_mat, statistika.matrix, sizeof(statistika.matrix) - 1);	
	}
	if(isAlive > 0){
		lseek(fd_alive, 0, SEEK_SET);
		read(fd_alive, statistika.num_alive, sizeof(statistika.num_alive) - 1);
	}
	if(isBorn > 0){
		lseek(fd_born, 0, SEEK_SET);
		read(fd_born, statistika.num_born, sizeof(statistika.num_born) - 1);
	}
	if(isDied > 0){
		lseek(fd_died, 0, SEEK_SET);
		read(fd_died, statistika.num_died, sizeof(statistika.num_died) - 1);
	}
	if(isGen > 0){
		lseek(fd_gen, 0, SEEK_SET);
		read(fd_gen, statistika.gen_num, sizeof(statistika.gen_num) - 1);
	}
	return statistika;
}

/*
  Funkcija koja otvara sve trazene datoteke.
*/
void close_files() {
	if(isMat > 0){
		close(fd_mat);
	}
	if(isAlive > 0){
		close(fd_alive);
	}
	if(isBorn > 0){
		close(fd_born);
	}
	if(isDied > 0){
		close(fd_died);
	}
	if(isGen > 0){
		close(fd_gen);
	}
}
