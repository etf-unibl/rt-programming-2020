#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>

#define M 10        
#define N 10

typedef struct
{
	int x;      //koordinate
	int y;      
    bool z;     //da li je nit startovana ili ne
}   CELL;
static int lap_counter=0;
static int staro_stanje[M][N];    
static int novo_stanje[M][N];
static pthread_mutex_t mutex;    
static sem_t semafor[M][N];      
static sem_t glavni_semafor;      
static CELL mapa[M][N]; 
static int pobudjenost[M][N];
//static pthread_barrier_t barrier;
    int sleep_time;

int odredjivanje_prioriteta(int x,int y){
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
{
for (int x = 0; x < M; x++){
		for (int y = 0; y < N; y++)
            staro_stanje[x][y]=novo_stanje[x][y];
    }
}
  
void restart() 
{ 
for (int x=0; x<=M;x++)
    for (int y=0; y<=N;y++) 
        pobudjenost[x][y]=0; 
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
				broj_zivih+=staro_stanje[pozicija.x][pozicija.y + 1];	//	
		} 
	    //Implementiramo pravila revolucije!
		if(staro_stanje[pozicija.x][pozicija.y]==1){  

			if(broj_zivih<2)        novo_stanje[pozicija.x][pozicija.y] = 0;
			else if(broj_zivih==3)	novo_stanje[pozicija.x][pozicija.y] = 1;
			else if(broj_zivih>3)   novo_stanje[pozicija.x][pozicija.y] = 0;
		}
            else if(broj_zivih==3)  novo_stanje[pozicija.x][pozicija.y] = 1;
	        else novo_stanje[pozicija.x][pozicija.y]=staro_stanje[pozicija.x][pozicija.y];
        if(pozicija.x + 1<=M-1 && (pozicija.y - 2)>=0 && pobudjenost[pozicija.x][pozicija.y]!=0){
				sem_post(&semafor[pozicija.x+1][pozicija.y-2]);
                pobudjenost[pozicija.x+1][pozicija.y-2]=1;
		}
            else
        { 
	      for(int x = 0; x < M; x++)
	      for(int y = 0; y < N; y++){
                if(odredjivanje_prioriteta(mapa[x][y].x,mapa[x][y].y) ==                          odredjivanje_prioriteta(pozicija.x,pozicija.y)+1 ){
							sem_post(&semafor[x][y]);
							pobudjenost[x][y] = 1;
							x = y = M+N;					
						}				
					}
			}
        lap_counter++; 
        if(lap_counter==N*M) sem_post(&glavni_semafor); 
		pthread_mutex_unlock(&mutex);
        if(lap_counter==N*M) sem_post(&glavni_semafor); 
           else sleep(1);
	}
}



int main(int argc, char **argv)
{ 

    char c;
    FILE *fp;
    if(argc != 3)
    {   //nedovoljan broj argumenata
    printf("Potrebno je unijeti ime fajla za pocetno stanje igre i vrijeme spavanja sistema\n");
        return -1;
    }
    if ( (sleep_time = atoi(argv[2]) )<=0)      
            { //atoi f-ja vraca 0 ako nije moguca validna konverzija u int
                    printf("Invalidan argument!\n"); return -1; 
            }
    if((fp=fopen(argv[1], "r"))==NULL)
    {
        printf("Neuspjesno otvaranje fajla.\n");
        return -1;
    }
    for(int i=0; i<M; i++)
        for(int j=0; j<N; j++)  //citamo pocetno stanje iz navedenog fajla 
            { 
                fscanf(fp,"%c\n",&c);
                if (c=='1') novo_stanje[i][j]=staro_stanje[i][j]=1; 
                else novo_stanje[i][j]=staro_stanje[i][j]=0; 
            }
    fclose(fp);             //zatvaranje fajla, obzirom da nam vise ne treba
	for(int x = 0; x < M; x++)
		for(int y = 0; y < N; y++)
			if(sem_init(&semafor[x][y],0,0))    // kreiramo mapu semafora (M*N)
				{   printf("Greska u kreiranju semafora!"); return -1;	} //kao i glavni semafor
	if(sem_init(&glavni_semafor,0,0)) { printf("Greska u kreiranju semafora!"); return -1;	}
	pthread_t matrix_threads[M][N];     // kreiramo M*N niti
	    
        for(int x = 0; x < M; x++)
		for(int y = 0; y < N; y++)
            {
    			mapa[x][y].x = x; mapa[x][y].y=y;   //postavljamo koordinate u date strukturu
    			if(pthread_create(&matrix_threads[x][y], NULL, thread_f, (void*)&mapa[x][y]))
                        { printf("Neuspjesno kreiranje niti!"); return -1; } 
        }       //startujemo tredove sa odgovarajucim parametrima
	if(sem_post(&semafor[0][0])){
		printf("Neuspjesno pokretanje!\n");
		return -1;	
	} 
	if(pthread_mutex_init(&mutex, NULL)){
		printf("Neuspjesno kreiranje mutexa!\n");	
		return -1;
	}
    //pthread_barrier_init(&barrier, NULL, N*M);
    update();
    print(staro_stanje);
    sem_post(&semafor[0][0]); 
	while (1) {
        lap_counter=0;      //za novu evoluciju resetujemo brojac niti koje su zavrsile svoj posao
		sem_wait(&glavni_semafor);  //pauziramo glavnu nit
        print(novo_stanje);         //ispisujemo trenutno stanje
        update();                   //prebacujemo novo stanje u staro
        sleep(sleep_time/1000);     //pauziramo sistem
		sem_post(&semafor[0][0]);   //pokrecemo prvu nit, koja ce pokrenuti ostale,  
	}                               //cime pokrecemo evoluciju 
        for(int x = 0; x < M; x++)
		for(int y = 0; y < N; y++)      //oslobadjanja semafora,mutexa
			if(sem_destroy(&semafor[x][y])) 
            {printf("Greska prilikom sem_destroy[%d][%d]!",x,y); return -1;}
	if(sem_destroy(&glavni_semafor))
            {printf("Greska prilikom sem_destroy!"); return -1;}
if (pthread_mutex_destroy(&mutex)) { 
        printf("Greska pri unistavanju mutex-a.\n");
		return -1;
        }
	return 0;
}
