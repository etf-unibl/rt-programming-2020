#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM 10
 
typedef struct xy{
	char x;
	char y;
	char wait;
}ARG;
 
 /*
  * sizeof(sem_t)=32
  * Puno memorije se zauzima samo za semafore (3232 B), ali program mora da radi na
  * sistemu sa oko 256 MB RAM pa gledano sa te strane nekoliko kB nije puno.
  * Sve se moglo uraditi i sa jednim semaforom ali bi onda potencijalno
  * izgubili na vremenu jer bi nakon zauzimanja resursa nit morala provjeriti stanje susjede i od 100 niti desavalo bi se da niti 
  * ciji nije red trose procesorsko vrijeme ne radeci nista. Jos gora situacija bi se desila da dodje do situacije da nit koja nije
  * na redu nakon provjere opet zauzme semafor i tako sprijecava druge niti da nesto urade.
  * Da bi se ovo sprijecilo niti bi trebale cekati razlicita vremena nakon oslobadjanja semafora sto opet uzima od brzine izvrsavanja.
  * Svi semafori su static i tako ih sve niti vide pa ako je vec prostor zauzet nema potrebe da se pokazivaci na neke prenose na stek
  * kao argumenti za niti i time zauzimaju nepotrebno memoriju.
  * 
  * Takodje sve druge strukture koje nose informacije o stanjima niti su static da nema nepotrebnih prenosa na stek, a niti
  * mogu da citaju sve a modifikuju podatke koje odgovaraju koordinatama koje ce im biti proslijedjene kao argument.
  * */

static sem_t go[NUM][NUM];
static sem_t start;
static pthread_t arr[NUM][NUM];
static pthread_t l;
static ARG t[NUM][NUM];

static char field[NUM][NUM];

static int end=0;

/*
 * Pomocu dvije petlje obidjemo sve sujede celije field[x][y] i bitovi promjenljive r predstavlja stanje okolnih celija.
 * */
void get_n(char* states,char x, char y){
	char r=1;
	char end1=x+1;
	char end2=y+1;
	char i,j;
	for(i=x-1;i<=end1;i++){
		for(j=y-1;j<=end2;j++){
			if(i<0 || i>=NUM || j<0 || j>=NUM){
				states[r]=0;
				r++;
			}
			else{
				if(i!=x || j!=y){
					if(field[i][j]==1 || field[i][j]==2){
						states[r]=1;
					}
					else{
						states[r]=0;
					}
					r++;
				}
			}
		}
	}
	return;
}
/*
 * Zahtjev je bio da se realizuje posebna funkcija koja prima niz sa 9 informacije o stanju celije i njenih susjeda i vraca 
 * informaciju o novom stanju celije.
 * */
char evolve(char* states){
	char r=0;
	char br=0;
	char i;
	for(i=1;i<9;i++){
		if(states[i]==1){
			br++;
		}
	}
	r=states[0];
	if(r==1){
		if(br!=2 && br!=3){
			r=2;
		}
	}
	else{
		if(br==3){
			r=3;
		}
	}
	return r;
}

//Funkcija koju izvrsava svaka nit koja predstavlja celiju.
void* thread(void* arg){
	ARG* a=(ARG*)arg;
	char states[9];
	char control=0;
	char i;
	while(1){
		/*
		 * Nit ceka da je svi susjedi na koje ceka obavijeste da moze da evoluira u novom ciklusu.
		 * Takodje kada zavrse evoluciju opet cekaju na semaforu. Ako se ustanovi da su 
		 * sve celije umrle ili je korisnik prekinuo izvrsavanje unosom karaktera na stdin
		 * niti se unistavaju i tako prekidaju sa radom.
		 * */
		
		for(i=0;i<a->wait;i++){
			sem_wait(&(go[a->x][a->y]));
		}
		 
		//Iako celije mogu da imaju cetiri stanja one koje nisu jos evoluirale mogu imati samo dva, 0 ili 1.
		if(field[a->x][a->y]==1){
			states[0]=1;
		}
		else{
			states[0]=0;
		}
		get_n(states,a->x,a->y);
		field[a->x][a->y]=evolve(states);
		/*
		 * Celija moze imati susjeda desno od sebe a da nema susjeda ispod i obrnuto, ali da bi imala susjeda
		 * dijagonalno od nje na desnoj strani mora imati susjeda desno i ispod sebe, tj. ne smije da bude u posljednjem redu ili koloni.
		 * Da ne bi opet provjeravali koristimo info sada da pamti da li su prva dva uslova ispunjena ili ne.
		 * */
		control=0;
		if(a->x+1<NUM){
			sem_post(&(go[a->x+1][a->y]));
			control++;
			//Samo ako postoji vrsta ispod trenutne celije provjeravamo da li postoji jedna lijeva dijagonalno od nje u toj vrsti.
			if(a->y-1>=0){
				sem_post(&(go[a->x+1][a->y-1]));
			}
		}
		if(a->y+1<NUM){
			sem_post(&(go[a->x][a->y+1]));
			control++;
		}
		if(control==2){
			sem_post(&(go[a->x+1][a->y+1]));
		}
		//Ova uslov ce samo zadnja celija ispuniti i sluzi za ponovno pokretanje ciklusa.
		if(control==0){
			sem_post(&start);
		}
	}
	return NULL;
}

//Ova funkcija racuna i vraca broj susjeda na koje mora cekati celija da bi dobila pravo da pocne sa izvrsavanjem u novom ciklusu.
//Koristi se pri inicijalizaciji niti.
char get_pn(char x, char y){
	char r=4;
	if(x==0){
		//sve celije u prvoj vrsti cekaju na jednog susjeda, sem prve koja nema susjednih
		//celija na koje ceka, ali zato ceka na semaforu signal za pocetak novog ciklusa
		r=r-3;
	}
	else{
		if(y==0){
			//U prvoj koloni sve celije cekaju na dvije susjedne celije, sem prve koja je vec obuhvacena prvim uslovom.
			r=r-2;
		}
		else{
			if(y==NUM-1){
				//U posljednjoj koloni sve celije cekaju na 3 susjedne celije, sem prve koja ceka na 1 i pripada 1. vrsti pa je 1. uslov
				//vec obuhvata.
				r=r-1;
			}
		}
	}
	//Sve ostale celije cekaju na 4 susjedne celije.
	return r;
}

char format_and_print_field(){
	char r=0;
	char i,j;
	for(i=0;i<NUM;i++){
		for(j=0;j<NUM;j++){
			if(field[i][j]==2){
				field[i][j]=0;
			}
			else{
				if(field[i][j]==3){
					field[i][j]=1;
				}
			}
			if(field[i][j]==1){
				r++;
				printf("  x ");
			}
			else{
				printf("  * ");
			}
		}
		printf("\n\n");
	}
	return r;
}

void* listen(void* arg){
    fgetc(stdin);
    end=2;
	return NULL;
}

int main(int argc, char* argv[]){
	char i,j;
	int wait;
	int ch;
	/*
	 * Pocetna konfiguracija ce se citati iz fajla koji je formatiran tako da sadrzi matricu karaktera NUMxNUM.
	 * Lakse je vizuelno mijenjati vec posebno namijenjen fajl sa nanom nego stalno unositi preko terminala 100 vrijednosti za celije
	 * ili unositi samo zive a da pri tom moramo imati mentanu sliku matrice u glavi.
	 * 
	 * Ime fajla se proslijedjuje kao argument komandne linije.
	 * */
	
	if(argc!=3){
		printf("Netacan broj argumenata proslijedjen!");
		return 1;
	}
	FILE* fp=fopen(argv[1],"r");
	if(fp==NULL){
		printf("Datoteka ne postoji ili je putanja do nje losa!\n");
		return 1;
	}
	
	//Citanje vrijednosti vremena koje ce se cekati. Jedinica vremena koja se koristi su milisekunde.
	for(i=0;argv[2][i]!='\0';i++){
		if(argv[2][i]<'0' || argv[2][i]>'9'){
			printf("Losa vrijednost vremena spavanja!\n");
			return 1;
		}
	}
	wait=atoi(argv[2]);
	
	for(i=0;i<NUM;i++){
		for(j=0;j<NUM;j++){
			ch=fgetc(fp);
			if(ch=='0'|| ch=='1'){
				field[i][j]=ch-'0';
			}
			else{
				printf("Format datoteke nije ocekivani!\n");
				fclose(fp);
				return 1;
			}
		}
		//cita enter
		fgetc(fp);
	}
	
	
	/*
	 * Inicijalizacija semafora.
	 * U dokumentaciji i u kodu za glibc pise da sem_init vraca 0 po uspjesnoj inicijalizaciji, a -1 ako dodje do greske.
	 * Akcije koje izazivaju vracanje -1 su:
	 * 		1. postavljanje vrijednosti semafora preko maksimalne dozvoljene,
	 * 		2. ako je pshared argument razlicit od nule, tj. semafor je diljeljen izmedju procesa, a sistem to ne podrzava.
	 * Posto se nijedan semafor ne dijeli sa nekim drugim procesom, vec sluze za komnikaciju izmedju niti u jedno procesu,
	 * nema potrebe da se vodi briga o ovoj gresci.
	 * A vrijednost semafora ne prelazi 4, maksimalna je 2^31-1, posto niti rade tako ne treba uzimati ni ovu gresku u obzir.
	 * Ako se program uspjesno ucitao u memoriju onda postoji dovoljno memorije za semafore posto se cuvaju u static matrici.
	 * Potrebno je paziti da se funkciji ne prosijedi pokazivac na vec inicijalizovani semafor. To se obezbjejuje petljom pa nema
	 * potrebe stalno provjeravati povratnu vrijednost.
	 * */
	for(i=0;i<NUM;i++){
		for(j=0;j<NUM;j++){
			sem_init(&(go[i][j]),0,0);
		}
	}
	
	sem_init(&start,0,0);
	
	/* 
	 * 
	 * pthread_create vraca 0 ako nije doslo do greske, a -1 ako je doslo do greske
	 * 
	 * Za razliku od sem_init gdje je potrebno obezbijediti samo dobre vrijednosti argumenata problemi oko stvaranja niti mogu da nastanu
	 * zbog uslova van programa, tj. nedostaka resursa na sistemu.
	 * Na primjer ako je broj trenutno aktivnih niti dostignuo maksimalnu vrijednost u /proc/sys/kernel/threads-max ili jednostavno nije
	 * bilo dovoljno memorije na steku.
	 * Sistem na kojem se ovaj program mora pokrenuti ima oko 256MB RAMa i ako provjerimo maksimalni broj niti sa
	 * cat /proc/sys/kernel/threads-max dobijamo 3954, cak i da su brojevi visi najsigunije bi bilo provjeriti jer nigdje nije
	 * zagarantovano sta ce na sistemu pored ovog programa biti pokrenuto.
	 * 
	 * Posto programu trebaju svih 100 niti, ako jedna ne uspije sa inicijalizacijom program ce se zavrsiti.
	 * */

	for(i=0;i<NUM;i++){
		for(j=0;j<NUM;j++){
			(t[i][j]).x=i;
			(t[i][j]).y=j;
			(t[i][j]).wait=get_pn(i,j);
			if(pthread_create(&(arr[i][j]),NULL,thread,(void*)(&t[i][j]))!=0){
				end=1;
				printf("Sistem nije uspio da napravi novu nit!\n");
				break;
			}
		}
		if(end!=0){
			break;
		}
	}
	if(end==0){
		end=pthread_create(&l,NULL,listen,NULL);
	}
	
	//Unistavanje svih niti koje su pravilno inicijalizovane u slucaju da jedna nije.
	if(end!=0){
		if(i==NUM){
			//ako je i stigao da se uveca na NUM onda su sve celijske niti inicijalizovane i j je takodje NUM
			//greska se onda desila kod listen niti i ta nit nije kreirana
			i=NUM-1;
			j=NUM-1;
		}
		else{
			//umanjujemo j jer se na inicijalizaciji [i][j] celije doslo do greske
			j--;
		}
		for(;i>=0;i--){
			while(j>=0){
				/*
				 * Petljom smo obezbijedili da se pthread_cancel poziva na nitima koje su inicijalizovane tako da zbog toga
				 * nece nastato greska. Dalje kako su inicijalizovane sa standardnim atributima mogu se prekinuti i pridruzivati
				 * pa nema potrebe da se provjerava da li je doslo do takvih gresaka.
				 * 
				 * pthread_join korisimo da osiguramo da ce se sve inizijalizovane niti zavrsiti prije nego sto se program zavrsi
				 * Ako se nit unistila, a pozvali smo join na nju, pthread_join ce se samo vratiti.
				 * */
				pthread_cancel(arr[i][j]);
				pthread_join(arr[i][j],NULL);
			}
			j=NUM-1;
		}
		return 1;
	}
	
	//Petlja u kojoj se kontrolise pocetak novog ciklusa.
	while(end==0){
		system("clear");
		sem_post(&(go[0][0]));
		sem_wait(&start);
		if(format_and_print_field()==0){
			//Ako se vrati nula nema zivih celija, potrebno je obavijestiti nite da prestanu raditi.
			end=1;
			break;
		}
		sleep(wait);
	}

    for(i=0;i<NUM;i++){
		for(j=0;j<NUM;j++){
			pthread_cancel(arr[i][j]);
			pthread_join(arr[i][j],NULL);
		}
	}

	for(i=0;i<NUM;i++){
		for(j=0;j<NUM;j++){
			//Samo ako je los pokazivac vraca gresku.
			sem_destroy(&(go[i][j]));
		}
	}

    if(end==1){
        pthread_cancel(l);
		pthread_join(l,NULL);
    }
	
	fclose(fp);
	return 0;
}
