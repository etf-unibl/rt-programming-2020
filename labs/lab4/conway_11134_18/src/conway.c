#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define M 10
#define N 10
#define PREKID 15

void* celija(void *);
int pocetno_stanje(pthread_t polje_matrice[M][N]);
int broj_zivih(int, int );
void print();
void copy();

typedef struct {
	int x, y;      //pozicije u matrici
	int evolucija; //da li je trenutno evoluirala ili treba doci na red
	int prioritet;
} POLJE;

static POLJE polje[M][N];

static int stara_matrica[M][N];
static int nova_matrica[M][N];
static pthread_mutex_t mutex;   //mutex za zakljucavanje polja
static sem_t semafor[M][N];     //semafor za svaku nit
static sem_t sem_sistem;        //semafor za main()

/*Funkcija postavlja pocetna stanja matrica i
kreira:
	-mutex
	-semafore za niti
	-main() semafor
	-polje
*/
int pocetno_stanje(pthread_t polje_matrice[M][N]) {
	int i, j, tmp, k = 1;
    char mat[M][N];   
    FILE *fp;

    if((fp=fopen("matrica.txt", "r"))==NULL)
    {
        printf("Neuspjesno otvaranje fajla.\n");
        return -1;
    }
    for(i=0; i<M; i++)
        for(j=0; j<N; j++)
            fscanf(fp,"%c\n",&mat[i][j]);

    for(i=0; i<M; i++)
    {
        for(j=0; j<N; j++)
        {
            if(mat[i][j]=='1')
                nova_matrica[i][j] = stara_matrica[i][j] = 1;  
            else if(mat[i][j]=='0')
                    nova_matrica[i][j] = stara_matrica[i][j] = 0;                  
        }  
    }
    fclose(fp);
       

	/*Kreiranje mutex-a*/
	tmp = pthread_mutex_init(&mutex, NULL);
	if (tmp) {
		printf("Greska kod kreiranja mutexa!\n");
		return -1;
	}

	/*Kreiranje semafora za niti i za main()*/
	for (i = 0; i < M; i++)
		for (j = 0; j < N; j++)
		{
			tmp = sem_init(&semafor[i][j], 0, 0);
			if (tmp) {
				printf("Greska kod kreiranja semafora!\n");
				return -1;
			}
		}
	tmp = sem_init(&sem_sistem, 0, 0);
	if (tmp) {
		printf("Greska kod kreiranja main semafora!\n");
		return -1;
	}
   	/*Kreiranje polja*/
	for (i = 0; i < M; i++)
	{
		for (j = 0; j < N; j++)
		{
			polje[i][j].x = i;
			polje[i][j].y = j;
			polje[i][j].prioritet = k + j;
			if (pthread_create(&polje_matrice[i][j], NULL, celija, (void*)&polje[i][j])) 
			{
				printf("Greska kod kreiranja niti.\n");
				return -1;
			}
		}
		k = k + 2;
	}

	return 0;
}
/*Ova matrica sluzi da lakse prebrojimo zive elemente posmatrane
  celije, tj. da lakse prebrojimo stanje komsijskih celija 
  posmatrane celije koje nemaju 8 susjednih.*/
static int tmp12[12][12] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				             { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
					         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
					         
/*Funkcija broji koliko ima zivih komsijskih celija*/
int broj_zivih(int x, int y)
{
	int i, j;
	int brojac = 0;
	/*Kopiranje matrice 10x10 u sredinu matrice 12x12*/
	for (i = 1; i <= 10; i++)
	{
		for (j = 1; j <= 10; j++)
		{
			tmp12[i][j] = stara_matrica[i - 1][j - 1];
		}
	}
    /*Brojanje zivih komsijskih celija*/
	for (i = -1; i <= 1; i++)
	{
		for (j = -1; j <= 1; j++)
		{
			brojac += tmp12[x + 1 + i][y + 1 + j];
		}
	}
	brojac -= tmp12[x + 1][y + 1]; 
	
	return brojac;
}
 
void* celija(void *polje_p)
{
    POLJE polje_n = *(POLJE*)polje_p;
    while(1)
    {
        
        sem_wait(&semafor[polje_n.x][polje_n.y]);
        
        pthread_mutex_lock(&mutex);
        int i, j;
        int zive=0;

        /*provjera zivih celija */
        zive = broj_zivih(polje_n.x, polje_n.y);
        
        /*Implementacija pravila zivota*/
        if ((stara_matrica[polje_n.x][polje_n.y] == 1) && (zive < 2)) 
            nova_matrica[polje_n.x][polje_n.y] = 0; 
        else if ((stara_matrica[polje_n.x][polje_n.y] == 1) && (zive > 3)) 
            nova_matrica[polje_n.x][polje_n.y] = 0; 
        else if ((stara_matrica[polje_n.x][polje_n.y] == 0) && (zive == 3)) 
            nova_matrica[polje_n.x][polje_n.y] = 1; 
        else
            nova_matrica[polje_n.x][polje_n.y] = stara_matrica[polje_n.x][polje_n.y];
        
        if((polje_n.x == M-1) && (polje_n.y==N-1))
        {
            for(i=0; i<N; i++)
                for(j=0; j<M; j++)
                    polje[i][j].evolucija = 0;
            sem_post(&sem_sistem);
        }
        else{
			if((polje_n.x + 1<=M-1) && (polje_n.y - 2>=0) && 
                !polje[polje_n.x+1][polje_n.y-2].evolucija)
			{
				sem_post(&semafor[polje_n.x+1][polje_n.y-2]);
				polje[polje_n.x+1][polje_n.y-2].evolucija = 1;		
			}
			else
			{ 
				for(i = 0; i < M; i++)
					for(j = 0; j < N; j++)
					{
						if(polje[i][j].prioritet == polje_n.prioritet+1)
						{
							sem_post(&semafor[i][j]);
							polje[i][j].evolucija = 1;
							i = PREKID;
							j = PREKID;
						}				
					}
			}
		}      
        pthread_mutex_unlock(&mutex);

        
    }
}

void print()
{
    int i, j;
    system("clear");
    fflush(stdout);
    for(i=0; i<M; i++)
    {
        for(j=0; j<N; j++)
        {
            if (nova_matrica[i][j] == 0) 
                printf(" . "); 
            else
                printf(" * "); 
        }
        printf("\n");
    }
    fflush(stdout);
}

void copy()
{
    int i, j;
    for(i=0; i<M; i++)
        for(j=0; j<N; j++)
            stara_matrica[i][j]=nova_matrica[i][j];
}

int main(int argc, char **argv)
{
	int tmp, i, j;
    int duzina_spavanja;
    
    if(argc != 2)
    {
        printf("Potrebno unjeti duzinu spavanja glavne niti u programu [u sekundama]!\n");
        return -1;
    }
    duzina_spavanja = atoi(argv[1]);

	pthread_t polje_matrice[M][N];

	tmp = pocetno_stanje(polje_matrice);
	if (tmp == -1) {
		printf("Neuspjesno kreiranje pocetnih stanja.\n");
		return -1;
	}

	tmp = sem_post(&semafor[0][0]);
	if (tmp) {
		printf("Neuspjesno startovanje prve niti.\n");
		return -1;
	}
    print();
    while(1)
    {
        sem_wait(&sem_sistem);
		print();              
		copy();
        print();
		sem_post(&semafor[0][0]);
		sleep(duzina_spavanja);
    }

	/*Unistavanje semafora i mutexa*/
	for (i = 0; i < M; i++)
		for (j = 0; j < N; j++)
		{
			tmp = sem_destroy(&semafor[i][j]);
			if (tmp) {
				printf("Greska pri unistavanju semafora [%d][%d].\n", i, j);
				return -1;
			}
		}
	tmp = sem_destroy(&sem_sistem);
	if (tmp) {
		printf("Greska pri unistavanju main() semafora.\n");
		return -1;
	}
	tmp = pthread_mutex_destroy(&mutex);
	if (tmp) {
		printf("Greska pri unistavanju mutex-a.\n");
		return -1;
	}

	return 0;
}
