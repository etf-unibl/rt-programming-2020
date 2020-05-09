#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define M 10        
#define N 10

typedef struct
{
	int x;      //koordinate svake celije 
	int y;      
}   CELL;
 
static int staro_stanje[M][N];    
static int novo_stanje[M][N];
static pthread_mutex_t mutex;    
static sem_t semafor[M][N];      
static sem_t glavni_semafor;      
static CELL mapa[M][N]; 
static int pobudjenost[M][N];

 
    int sleep_time;
    int ukupan_broj_zivih_celija; 
    int broj_umrlih_celija;
    int broj_rodjenih_celija;
    int broj_generacija=0;  

int odredjivanje_prioriteta(int x,int y){       //prioritet odredjujemo na osnovu koordinata celija
	int tmp = 1;
    int tmp2=0;
	for(int i=0;i<=x;i++){
		for(int j=0;j<=y;j++){
			tmp2 = tmp+j; 
		}       
		tmp+=2;
	}
    return tmp2;
}

void print(int matra[M][N])
{
system("clear");
	for (int x = 0; x < M; x++){
		for (int y = 0; y < N; y++)
			printf(" %d ", matra[x][y]);
		printf("\n");
	}
	fflush(stdout);
}

void update()
{   // kopiramo izracunata stanja celija, evoluiramo
for (int x = 0; x < M; x++){
		for (int y = 0; y < N; y++)
            staro_stanje[x][y]=novo_stanje[x][y];
    }
}

void racunaj() 
{ 
ukupan_broj_zivih_celija=0; 
broj_umrlih_celija=0;           //za svaku generaciju racunamo  nove podatke
broj_rodjenih_celija=0;
for (int x = 0; x < M; x++){
		for (int y = 0; y < N; y++)
                {
                    if (staro_stanje[x][y]==1 && novo_stanje[x][y]==0) broj_umrlih_celija++; 
                    if (staro_stanje[x][y]==0 && novo_stanje[x][y]==1) broj_rodjenih_celija++; 
                    if (novo_stanje[x][y]==1) ukupan_broj_zivih_celija++; 
                }
}
}

void restart() 
{ 
for (int x=0; x<=M;x++)
    for (int y=0; y<=N;y++) 
        pobudjenost[x][y]=0; 
}

char * pomocna_funkcija()
{ // upisivanje statistike u vidu formatiranih podataka  
char *x=(char*)malloc(5*sizeof(char*));
sprintf(&x[0],"%d",ukupan_broj_zivih_celija);
sprintf(&x[1],"%d",broj_umrlih_celija);
sprintf(&x[2],"%d",broj_rodjenih_celija);
if (broj_generacija!=0) sprintf(&x[3],"%d",broj_generacija/10); 
     else    sprintf(&x[3],"%d",0); 
if (broj_generacija!=0) sprintf(&x[4],"%d",broj_generacija%10); 
     else sprintf(&x[4],"%d",1); 
return x;
} 

void ispis_informacija(int fd) 
{ //citanje statistike i formatirani ispis 
char buf[5];
read(fd, buf, 5); 
printf("Ukupan broj zivih celija: %2c\n Broj umrlih celija: %2c\n Broj rodjenih celija: %2c\n Generacija:      %2c",buf[0],buf[1],buf[2]);
if(buf[3]!='0') printf("%c",buf[3]); 
printf("%c\n",buf[4]); 
}

void *thread_f(void* param){

	CELL pozicija = *(CELL*)param;

	    while(1){   //sve niti se stopiraju
    	sem_wait(&semafor[pozicija.x][pozicija.y]);
		pthread_mutex_lock(&mutex);  //samo jedna nit moze da udje u kriticnu sekciju 
		int broj_zivih=0;
		if( (pozicija.x - 1)>=0){
			broj_zivih+=staro_stanje[pozicija.x - 1][pozicija.y];	
			if(pozicija.y - 1>=0)
				broj_zivih+=staro_stanje[pozicija.x - 1][pozicija.y - 1];	
			if(pozicija.y + 1<N)
				broj_zivih+=staro_stanje[pozicija.x - 1][pozicija.y + 1];	
		}
		if(pozicija.y - 1>=0){
			broj_zivih+=staro_stanje[pozicija.x][pozicija.y - 1];
			if(pozicija.x + 1<M)
				broj_zivih+=staro_stanje[pozicija.x + 1][pozicija.y - 1]; 
		}	
		if(pozicija.x + 1<M){
			broj_zivih+=staro_stanje[pozicija.x + 1][pozicija.y];
			if(pozicija.y + 1<N)
				broj_zivih+=staro_stanje[pozicija.x + 1][pozicija.y + 1];		
		}
		if(pozicija.y + 1<N){
				broj_zivih+=staro_stanje[pozicija.x][pozicija.y + 1];	
		} 
	          
        /*Implementacija pravila zivota*/
        if ((staro_stanje[pozicija.x][pozicija.y] == 1) && (broj_zivih < 2)) 
            novo_stanje[pozicija.x][pozicija.y] = 0; 
        else if ((staro_stanje[pozicija.x][pozicija.y] == 1) && (broj_zivih > 3)) 
            novo_stanje[pozicija.x][pozicija.y] = 0; 
        else if ((staro_stanje[pozicija.x][pozicija.y] == 0) && (broj_zivih == 3)) 
            novo_stanje[pozicija.x][pozicija.y] = 1; 
        else
            novo_stanje[pozicija.x][pozicija.y] = staro_stanje[pozicija.x][pozicija.y];
        
        if((pozicija.x == M-1) && (pozicija.y==N-1))
        {
            restart();          //kada zadnja nit zavrsi posao
            broj_generacija++; // inkrementujemo broj evolucija 
            sem_post(&glavni_semafor);  // signaliziramo main niti da krene sa radom
        }
        else{
			if((pozicija.x + 1<=M-1) && (pozicija.y - 2>=0) && 
                !pobudjenost[pozicija.x+1][pozicija.y-2])
			{
				sem_post(&semafor[pozicija.x+1][pozicija.y-2]);
				pobudjenost[pozicija.x+1][pozicija.y-2] = 1;		
			}
			else
			{ 
				for(int i = 0; i < M; i++)
					for(int j = 0; j < N; j++)
					{
						if(odredjivanje_prioriteta(pozicija.x,pozicija.y) +1 == odredjivanje_prioriteta(i,j) )
						{
							sem_post(&semafor[i][j]);
							pobudjenost[i][j] = 1;
							i=j=N+M;
						}				
					}
			}
		}      
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char **argv)
{ 

    char c;
    FILE *fp;
	int fd=open("/proc/domaci_zadatak", O_RDWR);
    if(fd==-1) 
                {
                    printf("Greska pri otvaranju proc fajla"); 
                    return -1;
                 }  
    if(argc != 3)                               //nedovoljan broj argumenata
    {  
    printf("Potrebno je unijeti ime fajla za pocetno stanje igre i vrijeme spavanja sistema\n");
        return -1;
    }
    if ( (sleep_time = atoi(argv[2]) )<=0)      
            {                                   //atoi f-ja vraca 0 ako nije moguca validna konverzija u int
                    printf("Invalidan argument!\n"); return -1; 
            }
    if((fp=fopen(argv[1], "r"))==NULL)
    {
        printf("Neuspjesno otvaranje fajla.\n");
        return -1;
    }
    for(int i=0; i<M; i++)
        for(int j=0; j<N; j++)                   //citamo pocetno stanje iz navedenog fajla 
            { 
                fscanf(fp,"%c\n",&c);
                if (c=='1') novo_stanje[i][j]=staro_stanje[i][j]=1; 
                else novo_stanje[i][j]=staro_stanje[i][j]=0; 
            }
    fclose(fp);                                  //zatvaranje fajla, obzirom da nam vise ne treba
	for(int x = 0; x < M; x++)
		for(int y = 0; y < N; y++)
			if(sem_init(&semafor[x][y],0,0))                             // kreiramo mapu semafora (M*N)
				{   printf("Greska u kreiranju semafora!"); return -1;	} //kao i glavni semafor

	if(sem_init(&glavni_semafor,0,0)) { printf("Greska u kreiranju semafora!"); return -1;	}
	pthread_t matrix_threads[M][N];                                     // kreiramo M*N niti
	    
        for(int x = 0; x < M; x++)
		for(int y = 0; y < N; y++)
            {
    			mapa[x][y].x = x; mapa[x][y].y=y;                //postavljamo koordinate u date strukturu
    			if(pthread_create(&matrix_threads[x][y], NULL, thread_f, (void*)&mapa[x][y]))
                        { printf("Neuspjesno kreiranje niti!"); return -1; } 
        }                                                    //startujemo tredove sa odgovarajucim parametrima
	if(sem_post(&semafor[0][0])){
    printf("Neuspjesno pokretanje!\n");
		return -1;	
	} 
	if(pthread_mutex_init(&mutex, NULL)){
		printf("Neuspjesno kreiranje mutexa!\n");	
		return -1;
	}
 
    update();                            //azuriranje podataka za start
    restart();
    write(fd, pomocna_funkcija(), 5);   // upis pocetne statistike
    sem_post(&semafor[0][0]);             //pokretanje prve niti
	while (1) {
		sem_wait(&glavni_semafor);       //pauziramo glavnu nit
        print(novo_stanje);              //ispisujemo trenutno stanje celija
        ispis_informacija(fd);           //ispis statistike
        racunaj();                       //racunamo potrebne podatke o stanju celija
        update();                        //prebacujemo novo stanje u staro
        write(fd, pomocna_funkcija(),5); //upis statistike
		sem_post(&semafor[0][0]);       //pokrecemo prvu nit, koja ce pokrenuti ostale,  cime pokrecemo evoluciju 
        sleep(sleep_time*3);             //uspavamo sistem
	}                              

        for(int x = 0; x < M; x++)
		for(int y = 0; y < N; y++)      //oslobadjanja semafora,mutexa, fajl deskriptora
			if(sem_destroy(&semafor[x][y])) 
            {printf("Greska prilikom sem_destroy[%d][%d]!",x,y); return -1;}
	    if(sem_destroy(&glavni_semafor))
            {printf("Greska prilikom sem_destroy!"); return -1;}
        if (pthread_mutex_destroy(&mutex)) { 
        printf("Greska pri unistavanju mutex-a.\n");
		return -1;
        }
        close(fd); 
	return 0;
}
