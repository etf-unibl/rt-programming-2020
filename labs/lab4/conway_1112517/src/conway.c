#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#define N 10 //velicina tabele



int old_gen[N*N]  ; //stara stanja
int new_gen [N*N];  //nova stanja 
static sem_t semaphores[N*N]; //svaka celija ima svoj semafor
int sleeptime=0;
int arr[N*N];
void  create_neigh(int );
int update_cells(int *);
int matrix[3][3];
int states[10];

void copyold()   // kopira iz stare u novu generaciju
{
	int i;
	for(i=0;i<N*N;i++)
		old_gen[i]=new_gen[i];
}
int init_semaphores()	//inicijalizacija semafora
{
	int i;
	if(sem_init(&semaphores[0],0,1))
		return -1;
	for(i=N;i<((N*N));i++)
	{
		if(sem_init(&semaphores[i],0,0))
			return -1;
	}
	return 0;
}


int destroy_semaphores()               //unistavanje semafora 
{
	int i;
	for(i=0;i<N*N;i++)
		if(sem_destroy(&semaphores[i])) // ispitivanje da li je doslo do greske, ako jeste vraca -1
			return -1;
	return 0;
}

void setm(int matrix[3][3]) // status odgovarajucih susjeda stavlja u red
{
	
	states[0]=matrix[1][1];
	states[1]=matrix[0][0] ;
	states[2]=matrix[0][1];
	states[3]=matrix[0][2];
	states[4]=matrix[1][2];
	states[5]=matrix[2][2];
	states[6]=matrix[2][1];
	states[7]=matrix[2][0];
	states[8]=matrix[1][0];
	
}

void evolution(int d)
{ 
    
	int mat[3][3];
	int st[8];
	create_neigh(d); // za odredjenu poziiciju celije dodjelimo joj odgovarajuce susjede
	setm(mat);
	new_gen[d]=update_cells(st);
		
	
}
void  create_neigh(int id)

{
	
	
	if(id==0)       
	{
		matrix[0][0]=old_gen[N*N-1];   
		matrix[0][1]=old_gen[N*N -N];
		matrix[0][2]=old_gen[N*N-N+1];
		
		matrix[1][0]=old_gen[N-1];   
		matrix[2][0]=old_gen[2*N-1]; 
		
		matrix[1][1]=old_gen[0];  
		matrix[1][2]=old_gen[1];
		matrix[2][1]=old_gen[N];  
		matrix[2][2]=old_gen[N+1];  
	}
	else if(id==N-1) 
	{
		matrix[0][0]=old_gen[N*N-2];   
		matrix[0][1]=old_gen[N*N-1];   
		matrix[0][2]=old_gen[N*N-N]; 
		matrix[1][2]=old_gen[0];   
		matrix[2][2]=old_gen[N]; 
		matrix[1][0]=old_gen[N-2];   
		matrix[1][1]=old_gen[N-1];    
		matrix[2][1]=old_gen[2*N-2];
		matrix[2][0]=old_gen[2*N-3]; 
	}
	else if(id==N*N-N) 
	{
		matrix[0][0]=old_gen[N*N-N-1];   
		matrix[0][1]=old_gen[N*N- N-N];
		matrix[0][2]=old_gen[N*N-N-N+1];
		
		matrix[1][0]=old_gen[N*N-1];   
		matrix[2][0]=old_gen[N-1];
		matrix[1][1]=old_gen[N*N-N];  
		matrix[1][2]=old_gen[N*N -N+1];
		matrix[2][1]=old_gen[0];
		matrix[2][2]=old_gen[1]; 
	}
	else if(id==N*N-1)  
	{
		matrix[0][0]=old_gen[N*N-N -2];   
		matrix[0][1]=old_gen[N*N-N-1];
		matrix[0][2]=old_gen[N*N-2*N]; 
		matrix[1][2]=old_gen[N*N-N];  
		matrix[2][2]=old_gen[0];
		
		matrix[1][0]=old_gen[N*N-2];   
		matrix[1][1]=old_gen[id];
		matrix[2][1]=old_gen[N-1]; 
		matrix[2][0]=old_gen[N-2];
	}
	else if(id%N==0)  
	{
		matrix[0][0]=old_gen[id-1];   
		matrix[0][1]=old_gen[id-N];
		matrix[0][2]=old_gen[id-N+1];
	
		matrix[1][0]=old_gen[id+N-1];
		matrix[1][1]=old_gen[id];
		matrix[1][2]=old_gen[id+1];
	
		matrix[2][0]=old_gen[id+N+1];
		matrix[2][1]=old_gen[id+N];
		matrix[2][2]=old_gen[id+N+1];
	}
	else if(id%N==9) 
	{
		matrix[0][0]=old_gen[id-N-1];   
		matrix[0][1]=old_gen[id-N];
		matrix[0][2]=old_gen[id-N+1];
	
		matrix[1][0]=old_gen[id-1];
		matrix[1][1]=old_gen[id];
		matrix[1][2]=old_gen[id+1];
	
		matrix[2][0]=old_gen[id+N-1];
		matrix[2][1]=old_gen[id+N];
		matrix[2][2]=old_gen[id+N+1];
	}
	else if(id<N)  
	{
		matrix[0][0]=old_gen[id+N*N-N*N-1];   
		matrix[0][1]=old_gen[id+N*N-N];
		matrix[0][2]=old_gen[id+N*N-N+1];
	
		matrix[1][0]=old_gen[id-1];
		matrix[1][1]=old_gen[id];
		matrix[1][2]=old_gen[id+1];
	
		matrix[2][0]=old_gen[id+N-1];
		matrix[2][1]=old_gen[id+N];
		matrix[2][2]=old_gen[id+N+1];
	}
	else if(id>N*N-N) 
	{
		matrix[0][0]=old_gen[id-N-1];   
		matrix[0][1]=old_gen[id-N];
		matrix[0][2]=old_gen[id-N+1];
	
		matrix[1][0]=old_gen[id-1];
		matrix[1][1]=old_gen[id];
		matrix[1][2]=old_gen[id+1];
	
		matrix[2][0]=old_gen[id-N*N+N-1];
		matrix[2][1]=old_gen[id-N*N+N];
		matrix[2][2]=old_gen[id-N*N+N+1];
	}
	else                  
	{
		
		matrix[0][0]=old_gen[id-N-1];   
		matrix[0][1]=old_gen[id-N];
		matrix[0][2]=old_gen[id-N+1];
	
		matrix[1][0]=old_gen[id-1];
		matrix[1][1]=old_gen[id];
		matrix[1][2]=old_gen[id+1];
	
		matrix[2][0]=old_gen[id+N-1];
		matrix[2][1]=old_gen[id+N];
		matrix[2][2]=old_gen[id+N+1];
	}
	
	
	
}



int update_cells(int *states)
{
	int i, n=0;
	for (i=0; i<9; i++)
	{
		if (states[i]) n++; // gledamo koliko imamo zivih celija u nasem nizu
	}
	if(states[0]==1 && (n<2 || n>3)) states[0]=0; //celija umire ako ima manje od dva susjeda ili vise od 3
	 else if( n==3) states[0]=1; //ako ima tacno tri susjeda onda zivih
	 
	return states[0]; // program nam mora vracati stanje celije
	
	
}


void printmatrix(int* array) //ispis matrice na izlaz
{
	int i;
	int er = system("clear");
	if(er != 0){
		printf("Greska prilikom ciscenja ekrana!\n");
		return; 
	}
	for (i = 0; i < N*N; ++i)
	{
			printf("%d ", array[i]); 
		
		printf("\n");
	}
	fflush(stdout);
}




void* thread_function(void *param)
{
	
	int index=*(int *)param;
	int count=0;
	while(1)
	{
	sem_wait(&semaphores[index]);
	if(arr[index-1]==1&&arr[index-2]==1&&arr[index-2]==1&&arr[index-3]==1&&arr[index-4]==1) 
	{
	evolution(index);
	arr[index]=1;
	sem_post(&semaphores[index]);//aktivira ga 
	count++; 
	    if (count==N*N) //provjerava da li je doslo do kraja evolucije svih celija
	    {
	      printmatrix(old_gen);	 //ako jeste ispisuje novu generaciju i mijenja staru za novu
	      copyold();
	     }
	}
	else if(arr[index-1]==0) { //ispituje koja celija sa vecim prioritetom nije evoluirala
		evolution(index-1);
	arr[index-1]=1;
	sem_post(&semaphores[index-1]);
	count++; 
		
	}
	else if(arr[index-2]==0) {	evolution(index-2);
	arr[index-2]=1;
	sem_post(&semaphores[index-2]);
	count++;}
	else if(arr[index-3]==0) {	evolution(index-3);
	arr[index-3]=1;
	sem_post(&semaphores[index-3]);
	count++;}
	else if (arr[index-4]==0) {	evolution(index-4);
	arr[index-4]=1;
	sem_post(&semaphores[index-4]);
	count++;}
	usleep(sleeptime*1000);
	}		
}




int main(int argc, char *argv[])
{
    int i;

int index[100];
int error;
    pthread_t thread[N*N];
	init_semaphores();

	// Provjera argumenata
	if(argc != 2)
	{
		return printf("Greska, provjerite argumente!\n");
		return -1;
	}
	
	//Vrijeme spavanja 
	sleeptime = atoi(argv[1]);

     for(i=0;i<N*N;i++)                                     // slucajno generisem pocetnu populaciju
				old_gen[i]=((int)rand()%3==0)?1:0;
				
				
				
	for(i=0;i<N*N;i++)
			index[i]=i;                  
	for(i=0;i<N*N;i++)                                                        
			if((error=pthread_create(&thread[i],NULL,thread_function,(void*)&index[i])))    // kreniram niti, ispitujem da li je doslo do greske
			{	
				printf(" Thread is not created\n");
				return error;
			}

	
	sleep(sleeptime);
	
	getchar();
	destroy_semaphores(); 


	return 0;
}




