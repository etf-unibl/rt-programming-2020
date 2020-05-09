#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>  
#include <pthread.h>
#include <semaphore.h>
#include "getch.h"
#include "../../head/stat.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#define ALIVE 1
#define N 10
/* 
Nacin funkcionisanja:
 * 1 - (ziva celija)
 * 0 - !(ziva celija)
 * program generise slucajan broj zivih celija na slucajnim pozicijama	
 * celija [0][0] je komsija sa [0][1],[1][0],[1][1] i celijama sa druge strane "granice" [0][(N-1)],[1][(N-1)],[(N-1)][0],[(N-1)][1],[(N-1)][(N-1)] tj granicne celije su komsije 
  (i na osnovu njih se odredjuje stanje pri sledecoj evoluciji)
 * ovo pravilo vazi za sve celije na "granici"
 * [0][0] je celija sa najvecim prioritetom, zato sto je na matrici u gornjem desnom uglu i njeno se stanje prvo odredjuje 
*/

static sem_t *semaphores; /* svaka celija(nit) ima svoj semafor */ 
static sem_t qu_semaphore;
int **old_gen; 	/* matrica starog stanja */
int **new_gen;	/* matrica novog stanja */
unsigned int mili; /*vrijeme  nakon kojeg ce stanje biti azurirano*/
volatile char quit='s';
volatile stat_t dev_stat;
int fd;
/*provjerava da li je string broj,vraca -1 u slucaju da nije  i 0 ako jeste */
int is_numeric(char *string)
{
	int i=0;
	while(string[i]!='\0')
	{
		if(string[i]<'0' || string[i]>'9')
			return -1;
		i++;
	}
	return 0;
}
/*citanje fajla koji predstavlja stanje celija, vraca -1 u slucaju greske, u suprotnom 0*/
int read_map()
{
	FILE *fp;
	int i,j;
	char x;
	fp = fopen("map.txt","r");
	if(fp==NULL)
	{
		return -1;	
	}
	else
	{
		for(i = 0; i < N; i++)
		{
			for(j = 0; j < N; j++)
			{
			        x = fgetc(fp);
				 
				if(x=='1')
				{
					dev_stat.num_liv_cells++;
					old_gen[i][j]=new_gen[i][j] =ALIVE;
				}
				else
				{
					old_gen[i][j]=new_gen[i][j] =!ALIVE;
				}
			}
			x=fgetc(fp);
		}
		fclose(fp);
	}
       return 0;
}
/*funkcija za inicijalizaciju semafora, vraca -1 u slucaju greske, u suprotnom 0 */
int init_semaphores()	
{
	int i;
	semaphores=(sem_t *)malloc((((N*N)+1))*sizeof(sem_t));
	sem_init(&qu_semaphore,0,0);
	if(sem_init(&semaphores[N*N],0,1))
		return -1;
	for(i=0;i<((N*N));i++)
	{
		if(sem_init(&semaphores[i],0,0))
			return -1;
	}
	return 0;
}
/*funkcija za unistavanje svih semafora, vraca -1 u slucaju greske, u suprotnom 0*/
int destroy_semaphores()	
{
	int i;
	for(i=0;i<((N*N)+1);i++)
	{
		if(sem_destroy(&semaphores[i]))
			return -1;
	}
	sem_destroy(&qu_semaphore);
	return 0;
}
/*
 *funkcija koja vraca novo stanje (1 ili 0) celije na osnovu komsija i njenog trenutnog stanja
 *argument-niz cije vrijednosti mogu biti samo 1 ili 0 i predstavljaju stanje komsija celije
*/
int checkCell(int neighbors[])
{
	int i;
	int counter=0;
	for(i=0;i<8;i++)
	{
		if(neighbors[i])
			counter=counter+1;
	}
	
	if(counter==3 && neighbors[8]==!ALIVE)
	{
		dev_stat.num_born_cells++;
		return ALIVE;
	}
	if((counter==2 || counter==3) && neighbors[8]==ALIVE)
	{	
		return ALIVE;
	}
	if(neighbors[8]==ALIVE)
			dev_stat.num_dead_cells++;
	return !ALIVE;	 
}

/*
 *funkcija koja  bira komsije celije, poziva prethodnu funkciju i azurira matricu novog stanja 
 *argument-(int i, int j) predstavlja poziciju celije u matrici stanja	
*/
void  newState(int i,int j)	
{
	//nepregledno ali najkraci nacin da se odrede sve komsije
	int neighbors[]= 
	{
	old_gen[(i-1)==-1?(N-1):(i-1)][(j-1)==-1?(N-1):(j-1)],old_gen[(i-1)==-1?(N-1):(i-1)][j],old_gen[(i-1)==-1?(N-1):(i-1)][(j+1)%N],
	old_gen[(i+1)%N][(j-1)==-1?(N-1):(j-1)],old_gen[(i+1)%N][j],old_gen[(i+1)%N][(j+1)%N],
	old_gen[i][(j-1)==-1?(N-1):(j-1)],old_gen[i][(j+1)%N],old_gen[i][j]
	};
	
	new_gen[i][j]=checkCell(neighbors);
}

/*funkcija za ispis trenutnog stanja stanja*/
void print(int ** state)	
{
	int i,j;
	system("clear");
	printf("Za izlazak iz programa pritisnite 'q'\n");
	printf("Za prikaz stanja nakon iteracije pritisnite 's'\n");
	for(i=0;i<N;i++)
	{	printf("\n");
		for(j=0;j<N;j++)
		{
			if(state[i][j]==ALIVE)
			{
				printf(" \033[0;32m<>\033[0m");
			}
			else
			{
				printf(" \033[0;31m><\033[0m");
			}
		}
		
	}
	fflush(stdout);
}
void printState()
{
	printf("\nBroj zivih celija: %d\n",dev_stat.num_liv_cells);
	printf("Broj rodjenih celija %d\n",dev_stat.num_born_cells);
	printf("Broj umrlih celija %d\n",dev_stat.num_dead_cells);
	printf("Broj iteracije %d\n",dev_stat.gen_counter);
	printf("Za nastavak izvrsavanja pritisnite 'r'\n");
	fflush(stdout);
}
/*funkcija koju poziva nit stanja svaki put kad je zavrsena  evolucija svih celija*/
void* get_state()
{
	while(1)
	{
	sem_wait(&semaphores[((N*N))]);
	
	print(new_gen);
	
	if(quit=='s')
	{
		if(ioctl(fd, RD_CMD,(stat_t *)&dev_stat))
		{
			quit='q';	
			return NULL;	
		}
		
		printState();
		
		while(quit!='r')
		{
			if(quit=='q')
				sem_post(&qu_semaphore);
		}
	}	
	
	int **temp=old_gen;
	old_gen=new_gen;
	new_gen=temp;
	
	dev_stat.num_born_cells=0;
	dev_stat.num_dead_cells=0;
	dev_stat.gen_counter++;
	
	sem_post(&semaphores[0]);
	usleep(mili*1000);
	}		
}
/*
 * funkcija koju poziva nit koja predstavlja celiju (svaka celija ima svoj semafor)
 * argument- (void *arg) (kastuje se u (int *) predstavlja id niti
 */
void *life(void *arg)
{
	int id=(int )(*((int *)arg));
	int i=id/N; 	
	int j=id%N;
	while(1)
	{
		if(id==0)
		{		
			sem_wait(&semaphores[id]); 
			sem_post(&semaphores[id]);  
		}
		if(i==0)
		{
			sem_wait(&semaphores[id]); 
		}
		else 
		{
			sem_wait(&semaphores[id]);
			sem_wait(&semaphores[id]);
			if(j!=(N-1) && j!=0) 
			sem_wait(&semaphores[id]);
		}
		
		newState(i,j);
		
		if((j+1)<N)
		{
			sem_post(&semaphores[i*N+j+1]);
		}
		if((i+1)<N)
		{
			sem_post(&semaphores[(i+1)*N+j]);
			if((j-1)>=0)
			sem_post(&semaphores[(i+1)*N+j-1]);
		}
		if(id==((N*N)-1))
		{
			dev_stat.num_liv_cells=dev_stat.num_liv_cells+dev_stat.num_born_cells-dev_stat.num_dead_cells;
			if(ioctl(fd, WR_CMD,(stat_t *)&dev_stat))
			{
				printf("Gresk IOCTL\n");
				quit='q';
			}
			sem_post(&semaphores[((N*N))]);
			if(quit=='q')
				sem_post(&qu_semaphore);
		}
		
	}

}
/*funkcija za izlazak*/
void *quit_func(void *arg)
{
	changemode(1);
	while(1)
	{
  	while ( !kbhit() )
  	{
    		
  	}
 
  	quit = getchar();
 	if(quit=='q')
		{
			sem_wait(&qu_semaphore);
			printf("\n");
			changemode(0);
			close(fd);
			exit(0); 
		}
  
 
  
	}
	changemode(0);
}
//pomocna struktura prilikom inicijalizacije pozicija
typedef struct{
	int x; 
	int y;
}positions;
/*
 * u main funkciji se alociraju i inicijalizuju potrebni resursi, pokrecu niti i oslobadjaju resursi 
 * u slucaju greske funkcija vraca EXIT_FAILURE u suprotnom EXIT_SUCCESS
 */
int main(int argc,char *argv[])
{
	if(argc<3)
	{
		printf("Greska, nedovoljan broj argumenata\n");
		printf("Unesite argumente navedenim redom\n");
		printf("Vrijeme azuriranja stanja[ms]\n");
		printf("Samo jednu od opcija | 1-slucajno generisanje zivih celija | 2-citaj iz mape|\n");
		printf("Primjer:./conway.c 150 1\n");
		return EXIT_FAILURE;
	}
	
	if(is_numeric(argv[1]) || (argv[2][0]!='1' && argv[2][0]!='2'))
	{
		printf("Greska, nedozvoljen unos\n");
		printf("Unesite argumente navedenim redom\n");
		printf("Vrijeme azuriranja stanja[ms]\n");
		printf("Samo jednu od opcija | 1-slucajno generisanje zivih celija | 2-citaj iz mape|\n");
		printf("Primjer:./conway.c 150 1\n");
		return EXIT_FAILURE;		
	}
	
	if((fd=open("/dev/stat_device", O_RDWR))==-1)
	{
		printf("Greska prilikom otvaranja /dev/stat_device\n");
		printf("Pokusajte koristeci sudo ./conway_rpi miliseconds option ili provjerite da li je modul insertovan, sudo insmod sistem.ko\n");
		return EXIT_FAILURE;
	}
	
	printf("Za izlazak iz pritisnite q\n");
	printf("Za prikaz stanja nakon iteracije pritisnite s\n");
	
	mili=atoi(argv[1]);
	pthread_t cells[((N*N))];
	pthread_t state;
	pthread_t quit_thread;
	int index[((N*N))],i,j;
	old_gen=(int **) malloc(sizeof(int *)*N);
	new_gen=(int **)malloc(sizeof(int *)*N);
	dev_stat.num_liv_cells=0;
	dev_stat.num_born_cells=0;
	dev_stat.num_dead_cells=0;
	dev_stat.gen_counter=0;
	
	if(old_gen==NULL || new_gen==NULL)
	{
		printf("Nije moguce alocirati potrebnu memoriju\n");
		printf("Pokusajte ponovo pokrenuti program\n");
		return EXIT_FAILURE;
	}

	
	for(i=0;i<N;i++)
	{
		old_gen[i]=(int *)malloc(sizeof(int)*N);
		new_gen[i]=(int *)malloc(sizeof(int)*N);
		if(old_gen[i]==NULL || new_gen[i]==NULL)
		{
			printf("Nije moguce alocirati potrebnu memoriju\n");
			printf("Pokusajte ponovo pokrenuti program\n");
			return EXIT_FAILURE;
		}
	}
	
	for(i=0;i<N;i++)
		for(j=0;j<N;j++)
		{
			new_gen[i][j]=old_gen[i][j]=!ALIVE;
		}
	//random pozicije
	if(argv[2][0]=='1')
	{
	
		srand(time(NULL));
		int n=rand()%(N*N)+1;
		positions pos[n];
		int flag=0,x,y;
		
		for(i=0;i<n;i++)
		{
			pos[i]=(positions) {-1,-1};
			do
			{		
		 		x = rand()%N;
		 		y = rand()%N;
				flag=0;
				for(j=0;j<i;j++)
					if(pos[j].x==x && pos[j].y==y)
						flag=1; 
				pos[i]=(positions) {x,y};
			}while(flag);
			
			dev_stat.num_liv_cells++;
			new_gen[x][y]=old_gen[x][y]=ALIVE;
		}
	}
	else
	{	//pozicije iz mape
		if(read_map())
		{
			printf("Ne mogu otvoriti fajl\n");
			printf("Pokusajte ponovo pokrenuti program\n");
			return EXIT_FAILURE;
		}
	}
	//upis pocetnog stanja
	if(ioctl(fd, WR_CMD,(stat_t *)&dev_stat))
	{
		printf("Gresk IOCTL\n");
		return EXIT_FAILURE;
	}
	//incilijalizacija semafora
	if(init_semaphores())
	{
		printf("Greska prilikom inicijalizacije semafora\n");
		printf("Pokusajte ponovo pokrenuti program\n");
		return EXIT_FAILURE;
	}
	
	//kreriranje celija odnosno niti
	for(i=0;i<((N*N));i++)
	{
		index[i]=i;
		if(pthread_create(&cells[i],NULL,life,&index[i]))
		{
			printf("Greska prilikom kreiranja niti\n");
			printf("Pokusajte ponovo pokrenuti program\n");
			return EXIT_FAILURE;
		}
	}
	//kreiranje niti za izlaz
	if(pthread_create(&quit_thread,NULL,quit_func,NULL))
	{
		printf("Greska prilikom kreiranja niti\n");
		printf("Pokusajte ponovo pokrenuti program\n");
		return EXIT_FAILURE;
	}
	//kreiranje niti za provjeru stanja
	if(pthread_create(&state,NULL,get_state,NULL))
	{
		printf("Greska prilikom kreiranja niti\n");
		printf("Pokusajte ponovo pokrenuti program\n");
		return EXIT_FAILURE;
	}

	for(i=0;i<N;i++)
    	pthread_join(cells[i],NULL);
	
	pthread_join(state,NULL);
	
	if(destroy_semaphores())
	{
		printf("Greska prilikom unistavanja semafora\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
    
}
