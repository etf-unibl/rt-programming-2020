#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>


#define BROJ_CELIJA 100
#define B 10
char niz_znakova[200];

//struktura koja sadrzi sve potrebne informacije o jednoj celiji
typedef struct informacije_o_celiji
{
	int id;
	int bitni_susjedi[4];
	int nebitni_susjedi[4];
}informacije_o_celiji;
static sem_t semafori[BROJ_CELIJA];
static sem_t pocetak;
static sem_t kraj;
informacije_o_celiji info[BROJ_CELIJA];
int signal_za_kraj=0;
int dat_proc;

//Funkcija koja racuna pravila zivota iz igre za jednu celiju. Vraca izracunato stanje te celije, prima niz celija gdje je niz[4] data celija
char stanje_celije(char* niz)
{
	int i,br=0;
	for(i=0;i<9;++i)
		if(niz[i]=='o')
			br++;
	if(niz[4]=='x')
	{
		if(br==3)
			return 'o';
	}
	else
		if(br==3 || br==4)
			return 'o';
return 'x';

}

//Provjerava da li se moze izvrsiti evolucija, ako moze izvrsava se, ako ne moze nit ceka dok se ne bude mogla izvrsiti evolucija. Takodje nit ceka za signalizaciju
//da li se moze vrsiti nova evolucija. 
void* nit_funkcija(void *iden)
{
	informacije_o_celiji celija=*(informacije_o_celiji*)iden;
	int k = celija.id;
	int q,i;
	char niz[9];
	
	while(sem_wait(&pocetak)==0)
	{
		if(sem_trywait(&semafori[k])!=0)
		{
			
			if(info[k].bitni_susjedi[0]>=0) 
			{
				sem_wait(&(semafori[info[k].bitni_susjedi[0]]));
			}
			if(info[k].bitni_susjedi[1]>=0)
			{
				sem_wait(&(semafori[info[k].bitni_susjedi[1]]));
			}
				
			if(info[k].bitni_susjedi[2]>=0)
			{
				sem_wait(&(semafori[info[k].bitni_susjedi[2]]));
			}
			if(info[k].bitni_susjedi[3]>=0)
			{
				sem_wait(&(semafori[info[k].bitni_susjedi[3]]));
			}
			
			//FORMIRANJE NIZA ZA EVOLUCIJU
			for(q=0;q<4;++q)
			{
				if(info[k].bitni_susjedi[q] < 0)
					niz[q]='c';
				else
					niz[q]=niz_znakova[info[k].bitni_susjedi[q] + BROJ_CELIJA];
			}
			niz[4]=niz_znakova[k + BROJ_CELIJA];
			for(q=0;q<4;++q)
			{
				if(info[k].nebitni_susjedi[q] < 0)
					niz[5+q]='c';
				else
					niz[5+q]=niz_znakova[info[k].nebitni_susjedi[q] + BROJ_CELIJA];
			}
			
			niz_znakova[k]=stanje_celije(niz);
			
			sem_post(&semafori[k]);
			sem_post(&semafori[k]);
			sem_post(&semafori[k]);
			sem_post(&semafori[k]);
			sem_post(&semafori[k]);
		}
		else if(signal_za_kraj==1)
			return NULL;
		else
		{
			sem_post(&semafori[k]);
			sem_post(&pocetak);
		}

	}
	return NULL;
}

//kreira bitne i nebitne susjede od celije koja se nalazi na poziciji id u nizu celija. Te susjede upisuje u strukturu celije za koju se racunaju susjedi
void kreiranje_susjeda(int i, int j, int id)
{
	//BITNI SUSJEDI
	if((i-1)>=0 && (j-1)>=0)
		info[id].bitni_susjedi[0]=(i-1)*B+(j-1);
	else
		info[id].bitni_susjedi[0]=-1;
	if((i-1)>=0)
		info[id].bitni_susjedi[1]=(i-1)*B+j;
	else
		info[id].bitni_susjedi[1]=-1;
	if((j+1)<B && (i-1>=0))
		info[id].bitni_susjedi[2]=(i-1)*B+(j+1);
	else
		info[id].bitni_susjedi[2]=-1;
	if((j-1)>=0)
		info[id].bitni_susjedi[3]=i*B+(j-1);
	else
		info[id].bitni_susjedi[3]=-1;
	//NEBITNI SUSJEDI
	if((j+1)<B)
		info[id].nebitni_susjedi[0]=i*B+(j+1);
	else
		info[id].nebitni_susjedi[0]=-1;
	if((i+1)<B && (j-1)>=0)
		info[id].nebitni_susjedi[1]=(i+1)*B+(j-1);
	else
		info[id].nebitni_susjedi[1]=-1;
	if((i+1)<B)
		info[id].nebitni_susjedi[2]=(i+1)*B+j;
	else
		info[id].nebitni_susjedi[2]=-1;
	if((i+1)<B && (j+1)<B)
		info[id].nebitni_susjedi[3]=(i+1)*B+(j+1);
	else
		info[id].nebitni_susjedi[3]=-1;


}

//Brise ekran i ispisuje matricu celija na standardni izlaz
void ispis()
{
	int i,j;
	
	i = write(dat_proc,niz_znakova,2*BROJ_CELIJA);
	
	i=system("clear");
	printf("Kraj programa -> enter!\n");
	for(i=0;i<B;++i)
	{
		for(j=0;j<B;++j)
		{
			printf("%c ",niz_znakova[i*B+j]);
		}
		printf("\n");
	}
	printf("\n");
	
	i = read(dat_proc,niz_znakova,2*BROJ_CELIJA);
	
}

//Funkcija za nit koja ceka da celija u donjem desnom cosku izvrsi evoluciju, kad ona izvrsi evoluciju ispise se matrica celija, i signalizira se
//pocetak nove evolucije. Ili ako je skinuto bilo sta sa standardnog ulaza signalizira se svim nitima da je kraj igre
void* evolucija_funkcija(void *s)
{
	int spavanje=*(int *)s;
	int i,brojac;
	while(1)
	{
		
		if(sem_trywait(&semafori[BROJ_CELIJA-1])==0)
		{
			brojac=0;
			sem_post(&semafori[BROJ_CELIJA-1]);
			for(i=0;i<BROJ_CELIJA;++i)
				if( niz_znakova[i] == 'x' ) brojac++;
				else break;
			if(brojac == BROJ_CELIJA) 
			{
				sem_post(&kraj);
				
				ispis();
				
			}
			if(sem_trywait(&kraj)==0)
			{
				signal_za_kraj=1;
				for(i=0;i<BROJ_CELIJA;++i)
					sem_post(&pocetak);
				break;
			}
			else
			{
				
				ispis();
				
				sleep(spavanje);
				for(i=0;i<BROJ_CELIJA;++i)
					{sem_trywait(&semafori[i]);
					sem_trywait(&semafori[i]);
					sem_trywait(&semafori[i]);
					sem_trywait(&semafori[i]);
					sem_trywait(&semafori[i]);}
				for(i=0;i<BROJ_CELIJA;++i)
					sem_post(&pocetak);
			}			
		}
		usleep(5000);
	}
	
	
	return NULL;	
}


int main(int argc,char * argv[])
{
	pthread_t celije[BROJ_CELIJA],nit_za_evoluciju;
	int i,j,k,spavanje,kod_greske;
	FILE *dat;
	char znak;
	if(argc!=2)
	{
		printf("Nedovoljan broj argumenata komandne linije!\n");
		return 0;
	}
	if((dat=fopen("matrica.txt","r"))==NULL)
	{
		printf("Datoteka nije uspijesno otvorena\n");
		return 0;
	}
	for(k=0;k<BROJ_CELIJA;k++)
	{
		znak=fgetc(dat);
		if (znak =='x' || znak =='o')
		{
			niz_znakova[k]=znak;
			niz_znakova[k+BROJ_CELIJA]=znak;
			
			info[k].id=k;
		}
		else 
		{
			
			printf("Nisu dobri znakovi u datoteci!\n");
			fclose(dat);
			return 0;
		}		
	}
	fclose(dat);
	
	spavanje=atoi(argv[1]);
	
	if((dat_proc=open("/proc/stat_proc",O_RDWR))<0)
	{
		printf("Nije dobro otvorena datoteka /proc/stat_proc\n");
		return 0;
	}
	
	if(sem_init(&pocetak,0,BROJ_CELIJA)) return printf("Semafor pocetak nije keriran!\n");
	
	if(sem_init(&kraj,0,0)) return printf("Semafor kraj nije keriran!\n");
				
	ispis();
	
	sleep(spavanje);
	
	for(k=0;k<BROJ_CELIJA;k++)
	{	
		if(sem_init(&semafori[k],0,0)) return printf("%dti semafor nije keriran!\n",k);
		
	}
	
	
	for(k=0;k<BROJ_CELIJA;k++)
	{
		i=k/B;
		j=k%B;
		kreiranje_susjeda(i,j,k);
	
		kod_greske=pthread_create(&(celije[k]),NULL,nit_funkcija,(void*)(&info[k]));
		if(kod_greske) return printf("%dta nit nije kerirana! Kod greske %d\n",k,kod_greske);
	
	}

	kod_greske=pthread_create(&nit_za_evoluciju, NULL,evolucija_funkcija,(void *)(&spavanje));
	if(kod_greske) return printf("Nit za evoluciju nije kerirana! Kod greske %d\n",kod_greske);
	
	
	getchar();
	sem_post(&kraj);
	
	pthread_join(nit_za_evoluciju,NULL);
	
	for(k=0;k<BROJ_CELIJA;++k)
		pthread_join(celije[k],NULL);
	for(k=0;k<BROJ_CELIJA;++k)
		sem_destroy(&semafori[k]);
	sem_destroy(&pocetak);
	sem_destroy(&kraj);
	close(dat_proc);
 return 0;
}
