#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <fcntl.h>
#include "statistic.h"
#define N 10                            // velicina matrice

typedef struct priority                // struktura koja sadrzi prioritet i poziciju (id niti) svake celije 
{
		unsigned char position;
		unsigned char priority;
}PRIORITY;


unsigned char old_generation[N*N]  ; //polje gdje zive celije
unsigned char new_generation [N*N]; 
unsigned int end =100000;              // broj iteracija da ne bi trajalo beskonacno

sem_t semaphores[N*N];                 //svaka nit ima svoj semafor
PRIORITY p[N*N];

int second;
unsigned char count =0; 
int temp=0;
STATISTIC gameLife_statistic;
char buf[20];                                      // buffer
int file_desc;

void printAll( );                          // ispisuje stanje svih celija
void printStatistic();                  // ispis statistike
void sysfs();                             // upisivanje  u drajver

int init_semaphores()                  // inicijalizacija semafora
{
	int i,error;
	for(i=1;i<N*N;i++)
		if((error=sem_init(&semaphores[i],0,0))) 
			return error;
	if((sem_init(&semaphores[p[0].position],0,1)))		// ova ide prva
			return error;
	return 0;
}
int destroy_semaphores()               //unistavanje semafora 
{
	int i,error;
	for(i=0;i<N*N;i++)
		if((error=sem_destroy(&semaphores[i])))
			return error;
	return 0;
}
void get_priority()     // funkcija koja odredjuje prioritete, odnosno redoslijed po kom celije evoluiraju
{
	int i,j,first=1;
	for(i=0;i<N;i++)    
	{
		for(j=0;j<N;j++)
		{
			p[i*N+j].position=i*N+j;                      // u stvari cuvam id niti tj poziciju
			p[i*N+j].priority=first+j;
		}
		first+=2;
	}
}
void sort()            //funkcija koja sortira prioritete 
{	
		PRIORITY temp;
		int i,j;
		for(i=0;i<N*N-1;i++)
			for(j=i+1;j<N*N;j++)
				if(p[i].priority>p[j].priority)
				{
					temp=p[i];
					p[i]=p[j];
					p[j]=temp;
				}
}
void copy_to_old_generation()   // kopira iz stare u novu generaciju
{
	int i;
	for(i=0;i<N*N;i++)
		old_generation[i]=new_generation[i];
}

int get_number_of_live_cells()   // funkcija koja racuna broj zivih celija
{
	int i,number_live=0;
	for(i=0;i<N*N;i++)
		if(old_generation[i])
			number_live++;
	return number_live;
}

void  evolution(unsigned char  id)
{
	unsigned char matrix[3][3];
	/*
	napravio sam tako da celije na ivicama imaju susjede sa suprotne strane
	pa moram sve uslove ispitati jer nije isto ako je celija u cosku, sa strana ...
	*/
	
	if(id==0)        // gornji lijevi cosak
	{
		matrix[0][0]=old_generation[N*N-1];   // 99  popunim matricu celijama iz polja
		matrix[0][1]=old_generation[N*N -N]; //90
		matrix[0][2]=old_generation[N*N-N+1];  //91
		
		matrix[1][0]=old_generation[N-1];   // 9 popunim matricu celijama iz polja
		matrix[2][0]=old_generation[2*N-1];  //19
		
		matrix[1][1]=old_generation[0];   // popunim matricu celijama iz polja
		matrix[1][2]=old_generation[1];
		matrix[2][1]=old_generation[N];   //10
		matrix[2][2]=old_generation[N+1];   // 11 popunim matricu celijama iz polja
	}
	else if(id==N-1)  // 9  gornji desni cosak
	{
		matrix[0][0]=old_generation[N*N-2];   //98  popunim matricu celijama iz polja
		matrix[0][1]=old_generation[N*N-1];    //99
		matrix[0][2]=old_generation[N*N-N];  //90
		
		matrix[1][2]=old_generation[0];   // popunim matricu celijama iz polja
		matrix[2][2]=old_generation[N];  //10
		
		matrix[1][0]=old_generation[N-2];   //8 popunim matricu celijama iz polja
		matrix[1][1]=old_generation[N-1];    //9
		matrix[2][1]=old_generation[2*N-2];//18
		matrix[2][0]=old_generation[2*N-3]; //1
	}
	else if(id==N*N-N)  //donji lijevi cosak
	{
		matrix[0][0]=old_generation[N*N-N-1];   // 89 popunim matricu celijama iz polja
		matrix[0][1]=old_generation[N*N- N-N];//80
		matrix[0][2]=old_generation[N*N-N-N+1];//81
		
		matrix[1][0]=old_generation[N*N-1];   //99 popunim matricu celijama iz polja
		matrix[2][0]=old_generation[N-1];//9
		
		matrix[1][1]=old_generation[N*N-N];   // 90 popunim matricu celijama iz polja
		matrix[1][2]=old_generation[N*N -N+1];//91
		matrix[2][1]=old_generation[0];
		matrix[2][2]=old_generation[1]; 
	}
	else if(id==N*N-1)  //donji desni cosak
	{
		matrix[0][0]=old_generation[N*N-N -2];   //88 
		matrix[0][1]=old_generation[N*N-N-1];//89
		matrix[0][2]=old_generation[N*N-2*N]; //80
		
		matrix[1][2]=old_generation[N*N-N];   //90 
		matrix[2][2]=old_generation[0];
		
		matrix[1][0]=old_generation[N*N-2];   //98 
		matrix[1][1]=old_generation[id];
		matrix[2][1]=old_generation[N-1]; //9
		matrix[2][0]=old_generation[N-2];//8
	}
	else if(id%N==0)  // lijeva ivica
	{
		matrix[0][0]=old_generation[id-1];   
		matrix[0][1]=old_generation[id-N];
		matrix[0][2]=old_generation[id-N+1];
	
		matrix[1][0]=old_generation[id+N-1];
		matrix[1][1]=old_generation[id];
		matrix[1][2]=old_generation[id+1];
	
		matrix[2][0]=old_generation[id+N+1];
		matrix[2][1]=old_generation[id+N];
		matrix[2][2]=old_generation[id+N+1];
	}
	else if(id%N==9) //desna ivica
	{
		matrix[0][0]=old_generation[id-N-1];   
		matrix[0][1]=old_generation[id-N];
		matrix[0][2]=old_generation[id-N+1];
	
		matrix[1][0]=old_generation[id-1];
		matrix[1][1]=old_generation[id];
		matrix[1][2]=old_generation[id+1];
	
		matrix[2][0]=old_generation[id+N-1];
		matrix[2][1]=old_generation[id+N];
		matrix[2][2]=old_generation[id+N+1];
	}
	else if(id<N)  //gornja ivica
	{
		matrix[0][0]=old_generation[id+N*N-N*N-1];   
		matrix[0][1]=old_generation[id+N*N-N];
		matrix[0][2]=old_generation[id+N*N-N+1];
	
		matrix[1][0]=old_generation[id-1];
		matrix[1][1]=old_generation[id];
		matrix[1][2]=old_generation[id+1];
	
		matrix[2][0]=old_generation[id+N-1];
		matrix[2][1]=old_generation[id+N];
		matrix[2][2]=old_generation[id+N+1];
	}
	else if(id>N*N-N) // donja ivica
	{
		matrix[0][0]=old_generation[id-N-1];   
		matrix[0][1]=old_generation[id-N];
		matrix[0][2]=old_generation[id-N+1];
	
		matrix[1][0]=old_generation[id-1];
		matrix[1][1]=old_generation[id];
		matrix[1][2]=old_generation[id+1];
	
		matrix[2][0]=old_generation[id-N*N+N-1];
		matrix[2][1]=old_generation[id-N*N+N];
		matrix[2][2]=old_generation[id-N*N+N+1];
	}
	else                     // ako nije granicni slucaj
	{
		
		matrix[0][0]=old_generation[id-N-1];   
		matrix[0][1]=old_generation[id-N];
		matrix[0][2]=old_generation[id-N+1];
	
		matrix[1][0]=old_generation[id-1];
		matrix[1][1]=old_generation[id];
		matrix[1][2]=old_generation[id+1];
	
		matrix[2][0]=old_generation[id+N-1];
		matrix[2][1]=old_generation[id+N];
		matrix[2][2]=old_generation[id+N+1];
	}
	// ako je suma svih polja 3 stanje celije je 1, ako je suma 4 zadrzava stanje, inace umire 
	unsigned char sum=matrix[0][0]+matrix[0][1]+matrix[0][2]+matrix[1][0]+matrix[1][1]+matrix[1][2]+matrix[2][0]+matrix[2][1]+matrix[2][2];
	if(sum!=4)  //ako je 4 zadrzava stanje 
	{
		if(sum==3)
		{
			matrix[1][1]=1;      // radja se celija
			gameLife_statistic.number_born++;
		}
		else
		{
			matrix[1][1]=0;      // celija umire
			gameLife_statistic.number_died++;
		}
		//matrix[1][1]=(sum==3)?1:0;
	}
	old_generation[id]=matrix[1][1];// jedino se ovo polje mijenja 
	gameLife_statistic.number_live=get_number_of_live_cells();    // broj trenutno zivih celija
	count++;                                   
	if(count ==N*N)
	{
		count =0;
		gameLife_statistic.number_generations++;        // nova generacija pocinje
		printAll();
		sysfs();                                                               // upisivanje u drajver
		sleep(second);
	}
	sem_post(&semaphores[p[count].position]);   // aktivira sledecu celiju iz sortirane strukture
}

void printStatistic()
{
	printf("\x1b[0m");
	printf("         ------------- Statistic -----------\n");
	printf("             Number of born cells:%d\n",gameLife_statistic.number_born);
	printf("             Number of died cells:%d\n",gameLife_statistic.number_died);
	printf("             Number of live cells:%d\n",gameLife_statistic.number_live);
	printf("             Number of generations:%d\n",gameLife_statistic.number_generations);
	printf("         ------------------------------------\n");
}

void printAll()
{	
		int i;
		system("clear");
		for(i=0;i<N*N;i++)
		{                                         //   zeleni  X-ziva    :  crveni X- mrtva
			printf("%s",old_generation[i]?"\x1b[1;32m X":"\x1b[1;31m X");
			if((i+1)%N==0)
				printf("\n");
			fflush(stdout);
		}
		printStatistic();
}
void* gameLife(void* param)
{
	unsigned char id=*(unsigned char*)param;
	//while(end--)   // da bi ogranicio trajanje programa
	while(end)  
	{
			sem_wait(&semaphores[id]);
			evolution(id);	
	}
	pthread_exit(NULL);
}

void sysfs()
{
    lseek(file_desc, 0, SEEK_SET);  
    sprintf(buf,"%d %d %d %d",gameLife_statistic.number_born,gameLife_statistic.number_died,gameLife_statistic.number_live,gameLife_statistic.number_generations);
    write(file_desc, buf, strlen(buf)+1);    
}

int main(int  argc,char* argv[])   
{
		int error;
		if(argc !=2)	
		{
			printf(" Set time for sleep in second \n");
			return -1;  
		}
		file_desc = open("/sys/kernel/stat_sysfs/stat_value", O_RDWR);
		if(file_desc < 0)
		{
			printf("Can't open /sys/kernel/stat_sysfs/stat_value!\n");
			printf(" Try: sudo insmod sistem.ko  or\n sudo chmod 666 /sys/kernel/stat_sysfs/stat_value\n");
			return -1;
		}

		second=atoi(argv[1]); 
		gameLife_statistic.number_generations=0;
		
		unsigned char index[N*N];
		int i;
		
		printf(" Press ENTER at any time to end program\n");
		sleep(2);
		get_priority();                                   //izracunam prioritete
		sort();                                              // sortiram
		if((error=init_semaphores()))
		{
			printf(" Error, semaphores is not initialized\n");
			close(file_desc);
			return error;
		}
		pthread_t  thread[N*N] ;                            //     100 celija
		for(i=0;i<N*N;i++)                                     // slucajno generisem pocetnu populaciju
				old_generation[i]=((unsigned char)rand()%3==0)?1:0;
		for(i=0;i<N*N;i++)
			index[i]=i;                  
		for(i=0;i<N*N;i++)                                                        
			if((error=pthread_create(&thread[i],NULL,gameLife,(void*)&index[i])))    // kreniram niti
			{	
				printf(" Thread is not created\n");
				close(file_desc);
				return error;
			}
		getchar();   // da se main ne zavrsi prije ostalih niti 
		
		if((error=destroy_semaphores()))
		{
			printf(" Error, semaphores is not destroyed!\n");
			close(file_desc);
			return error;
		}
		close(file_desc);
		printf("END\n");
		return 0;	
}

