#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>

#define DIM 10

typedef struct
{
	int x, y, priority;
	bool alive;
} CELL;

static sem_t semaphore[DIM][DIM];
static sem_t semaphore_start;
static pthread_mutex_t mutex;

static bool old_matrix[DIM][DIM];
static bool evoluted_matrix[DIM][DIM];
static CELL matrix[DIM][DIM];

void printing_matrix(bool[DIM][DIM]);
void copying_matrix();
void set_priority ();
int create_cell(pthread_t[DIM][DIM]);
void* start_routine(void*);
void evolve_next_gen(int, int, int);

int main(int argc, char* args[])
{
	if (argc!=3)
		return printf("Greska prilikom unosa argumenata!\n");
	
	FILE *f=fopen(args[1], "r");
	if(f=NULL)
		return printf("Greska prilikom otvaranja fajla!\n");
	else
	{
		printf("Citanje i prikaz matrice iz fajla...\n");
		for(int i=0; i<DIM; ++i)
		{
			for(int j=0; j<DIM; ++j)
			{
				int c=fgetc(f);
				if((char)c=='\n')
					--j;
				else
					while(c!=EOF)
						old_matrix[i][j]=evoluted_matrix[i][j] = i==0x31 ? true : false;
			}
		}
		
		printing_matrix(evoluted_matrix);
	}
	fclose(f);
	
	int sleep_time=atoi(args[2]);
	
	for(int i=0; i<DIM; ++i)
	{
		for(int j=0; j<DIM; ++j)
		{
			if(sem_init(&semaphore[i][j], 0, 0))
				return printf("GRESKA za semafor!\n");
		}
	}
	if(sem_init(&semaphore_start, 0, 0))
		return printf("GRESKA za STARTsemafor!\n");
	
	pthread_t matrix_threads[DIM][DIM];
	create_cell(matrix_threads);
	if(sem_post(&semaphore[0][0]))
	{
		return printf("GRESKA PRI POKRETANJU!\n");
	}
	pthread_mutex_init(&mutex, NULL);
	
	while(1)
	{
		sem_wait(&semaphore_start);
		printing_matrix(evoluted_matrix);
		copying_matrix();
		sem_post(&semaphore[0][0]);
		sleep(sleep_time/1000);
	}
	
	pthread_mutex_destroy(&mutex);
	for(int i=0; i<DIM; ++i)
		for(int j=0; j<DIM; ++j)
			sem_destroy(&semaphore[i][j]);
	sem_destroy(&semaphore_start);
	
	return 0;
}

void printing_matrix(bool m[DIM][DIM])
{
	system("clear");
	for (int i = 0; i < DIM; ++i){
		for (int j = 0; j < DIM; ++j)
			printf("%c", m[i][j] ? 0x31 : 0x30);
		printf("\n");
	}
	fflush(stdout);
}

void copying_matrix()
{
	for (int i = 0; i < DIM; ++i)
		for (int j = 0; j < DIM; ++j)
			old_matrix[i][j]=evoluted_matrix[i][j];	
}

void set_priority ()
{
	int initial=1;
	for(int i=0; i<DIM; ++i)
	{
		for(int j=0; j<DIM; ++j)
		{
			matrix[i][j].priority=initial+j;
		}
		initial+=2;
	}
}

int create_cell(pthread_t matrix_t[DIM][DIM])
{
	set_priority();
	
	for(int i=0; i<DIM; ++i)
	{
		for(int j=0; j<DIM; ++j)
		{
			matrix[i][j].x=i;
			matrix[i][j].y=j;
			if(pthread_create(&matrix_t[i][j], NULL, start_routine, (void*)&matrix[i][j]))
				return printf("Neuspjesno kreiranje niti!\n");
		}	
	}
	return 0;
}

void evolve_next_gen(int num, int x, int y)
{	
	if(old_matrix[x][y])
	{
		if(num==2 || num==3)
			evoluted_matrix[x][y]=true;
		else
			evoluted_matrix[x][y]=false;
	}
	else
	{
		if(num==3)
			evoluted_matrix[x][y]=true;
	}
}

void* start_routine(void* arg)
{
	CELL new_cell=*(CELL*) arg;
	
	while(true)
	{
		sem_wait(&semaphore[new_cell.x][new_cell.y]);
		
		pthread_mutex_lock(&mutex);
		
		int num_of_living=0; 
		
		
		if(new_cell.x>0)
		{
			num_of_living+=old_matrix[new_cell.x-1][new_cell.y];
			
			if(new_cell.y>0)
				num_of_living+=old_matrix[new_cell.x-1][new_cell.y-1];
		
			if(new_cell.y<(DIM-1))
				num_of_living+=old_matrix[new_cell.x-1][new_cell.y+1];			
		}
		
		if (new_cell.y>0)
		{
			num_of_living+=old_matrix[new_cell.x][new_cell.y-1];
			
			if(new_cell.x<(DIM-1))
				num_of_living+=old_matrix[new_cell.x+1][new_cell.y+1];		
		}
		
		if(new_cell.x<(DIM-1))
		{
			num_of_living+=old_matrix[new_cell.x+1][new_cell.y];
			
			if(new_cell.y<(DIM-1))
				num_of_living+=old_matrix[new_cell.x+1][new_cell.y+1];
		}
		
		if(new_cell.y<(DIM-1))
		{
			num_of_living+=old_matrix[new_cell.x][new_cell.y+1];
		}
		
		evolve_next_gen(num_of_living, new_cell.x, new_cell.y);		
		
		if(new_cell.x==DIM-1 && new_cell.y==DIM-1)
		{
			for(int i=0; i<DIM; ++i)
				for(int j=0; j<DIM; j++)
					matrix[i][j].alive=false;
			sem_post(&semaphore_start);
		}
		else
		{
			if((new_cell.x+1)<DIM-1 && (new_cell.y-2)>=2 && !matrix[new_cell.x+1][new_cell.y-2].alive)
			{
				sem_post(&semaphore[new_cell.x+1][new_cell.y-2]);
				matrix[new_cell.x+1][new_cell.y-2].alive=true;
			}
			else
			{
				for(int i=0; i<DIM; ++i)
					for(int j=0; j<DIM; j++)
					{
						if(matrix[i][j].priority == new_cell.priority+1)
						{
							sem_post(&semaphore[i][j]);
							matrix[i][j].alive=true;
							i=j=DIM;
						}
					}
			}
		}
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}
