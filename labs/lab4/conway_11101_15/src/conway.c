#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>  
#include <pthread.h>
#include <semaphore.h>
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
int **old_gen; 	/* matrica starog stanja */
int **new_gen;	/* matrica novog stanja */
unsigned int mili; /*vrijeme  nakon kojeg ce stanje biti azurirano*/

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
int read_map(int **state)
{
	FILE *fp;
	int i,j;
	char x;
	fp = fopen("../map.txt","r");
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
				state[i][j] = x=='1' ? ALIVE : !ALIVE;
				
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
	if(sem_init(&semaphores[0],0,1))
		return -1;
	for(i=N;i<((N*N)+1);i++)
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
	if(counter<2 || counter>3)
		return !ALIVE;
	if(counter==3 && neighbors[8]==0)
		return ALIVE;
	if((counter==2 || counter==3) && neighbors[8]==1)
		return ALIVE;
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
/*funkcija koju poziva nit stanja svaki put kad je zavrsena  evolucija svih celija*/
void* get_state()
{
	while(1)
	{
	sem_wait(&semaphores[((N*N))]);
	print(new_gen);	
	int **temp=old_gen;
	old_gen=new_gen;
	new_gen=temp;
	sem_post(&semaphores[0]);
	usleep(mili*1000);
	}		
}
/*
 * funkcija koju poziva nit koja predstavlja celiju (svaka celija ima svoj semafor)
 * argument- (void *arg) (kastuje se u (int *) predstavlja id niti
 */
void *thread(void *arg)
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
			sem_post(&semaphores[((N*N))]);
		}
		
	}

}
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
	mili=atoi(argv[1]);
	pthread_t cells[((N*N))];
	pthread_t state;
	int index[((N*N))],i,j;
	old_gen=(int **) malloc(sizeof(int *)*N);
	new_gen=(int **)malloc(sizeof(int *)*N);
	
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
			old_gen[i][j]=!ALIVE;
		}
	if(argv[2][0]=='1')
	{
	//bira slucajnih 30 pozicija na kojima cemo imati zive celije
	srand(time(NULL));
	int n=rand()%(N*N)+1;
	for(i=0;i<n;i++)
	{
		int x = rand()%N;
		int y =	rand()%N;
		old_gen[x][y]=ALIVE;
	}
	}
	else
	{
	if(read_map(old_gen))
	{
		printf("Ne mogu otvoriti fajl\n");
		printf("Pokusajte ponovo pokrenuti program\n");
		return EXIT_FAILURE;
	}
	}
	print(old_gen);
	
	if(init_semaphores())
	{
		printf("Greska prilikom inicijalizacije semafora\n");
		printf("Pokusajte ponovo pokrenuti program\n");
		return EXIT_FAILURE;
	}
	
	for(i=0;i<((N*N));i++)
	{
		index[i]=i;
		if(pthread_create(&cells[i],NULL,thread,&index[i]))
		{
			printf("Greska prilikom kreiranja niti\n");
			printf("Pokusajte ponovo pokrenuti program\n");
			return EXIT_FAILURE;
		}
	}
	
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

	for(i=0;i<N;i++)
	{
		free(old_gen[i]);
		free(new_gen[i]);
	}

	free(old_gen);
	free(new_gen);
	free(semaphores);
    
	return EXIT_SUCCESS;
}
