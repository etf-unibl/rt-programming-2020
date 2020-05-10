#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 

#define MAX 10 

char getch(void);
char FILENAME[]="matrix.txt";
int SLEEP=0;
int matrix[MAX][MAX];
volatile char done=0x0;
sem_t semaphore[MAX][MAX],sem;
FILE *fp;

/*  Struktura koja opisuje poziciju celije u matrici.   */
typedef struct position
{
     int row;
     int column;
} POSITION;

/*  Ispis na prazan ekran.  */
void clear_screen(void)
{
	const char* CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
	write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}
/*  Citanje stanja celija iz datoteke. Celije su opisane brojevima 1(ziva) i 0(mrtva).  */
int readMatrix()
{
    char c;
    int x;
    if((fp=fopen(FILENAME,"r"))==0)
    {
        printf("File not open.\n");
        return -1;
    }else
    {
        for(int i=0; i<MAX; i++)
        {
            for(int j=0; j<MAX; j++)
            {
                c=fgetc(fp);
                x=c-'0';
                if(x<0 || x>1)
                    {
                        printf("Error in file.\n");
                        return -1;
                    }
                matrix[i][j]=x;
            }
        }
    fclose(fp);
    }
    return 0;
}
/*  Ispis matrice koja predstavlja tabelu sa celijama na standardni izlaz.  */
void printMatrix()
{
    clear_screen();
    for(int i=0; i<MAX; i++)
    {
        for(int j=0; j<MAX; j++)
        printf("%d ", matrix[i][j]);
        printf("\n");
    }       
    printf("\n"); 
}
/*  Funkcija koja vraca broj zivih komsija celije koja se nalazi na poziciji (i,j) u tabeli celija. */
int numberOfLivingNeighbors(int i, int j)
{
    int n=0;
    if(i<0 || i>=MAX || j<0 || j>=MAX) 
        return -1;

    if((i-1)>=0 && (j-1)>=0)
        n=n+matrix[i-1][j-1];
    if((i-1)>=0)
        n=n+matrix[i-1][j];
    if((i-1)>=0 && (j+1)<MAX)
        n=n+matrix[i-1][j+1];
    if((j-1)>=0)
        n=n+matrix[i][j-1];
    if((j+1)<MAX)
        n=n+matrix[i][j+1];
    if((i+1)<MAX && (j-1)>=0)
        n=n+matrix[i+1][j-1];
    if((i+1)<MAX)
        n=n+matrix[i+1][j];
    if((i+1)<MAX && (j+1)<MAX)
        n=n+matrix[i+1][j+1];
    return n;
}
/*  Funkcija vraca poziciju celije koja je na redu za evolucioni ciklus nakon sto je
    celija sa pozicijom (a,b) u tabeli celija upravo zavrsila svoj evolucioni ciklus.  */
POSITION nextElement(int a,int b)
{
    POSITION tmp={0,0};
    if(a==MAX-1 && b==MAX-1)
        return tmp;
    if(a==MAX-1 && b==0)
    {
        tmp.row=1;
        tmp.column=MAX-1;
        return tmp;
    }
    if(a+b<MAX) //elements above and on second diagonal
    {
       if(b==0)
        {
            tmp.row=0; 
            tmp.column=a+b+1;
        }
        else
        {
            tmp.row=a+1;
            tmp.column=b-1;
        }
    }
    else if(a+b>=MAX)    //elements below second diagonal
    {
        if(a==(MAX-1))
        {
            tmp.row=b+1;
            tmp.column=MAX-1;
        }
        else
        {
            tmp.row=a+1;
            tmp.column=b-1;
        }
    }    
        return tmp;
}
/*  Nit koja predstavlja celiju. Po pravilima igre odredjuje se novo stanje celije. 
    Novo stanje celije se upisuje u datoteku na odgovarajucu poziciju. */
void* thread(void* arg) 
{ 
    POSITION tmp=*(POSITION*)arg;
    while(done!='q')
    {    
        sem_wait(&semaphore[tmp.row][tmp.column]); 
        
        int n=numberOfLivingNeighbors(tmp.row,tmp.column);
        if((n==2 || n==3 ) && matrix[tmp.row][tmp.column]==1)
            {
                fseek(fp,tmp.row*MAX+tmp.column,0);
                fputc('1',fp);
            }
        else if (matrix[tmp.row][tmp.column]==0 && n==3)
            {
                fseek(fp,tmp.row*MAX+tmp.column,0);
                fputc('1',fp);
            }
        else
            {
                fseek(fp,tmp.row*MAX+tmp.column,0);
                fputc('0',fp);
            }
        tmp=nextElement(tmp.row,tmp.column);
        sem_post(&semaphore[tmp.row][tmp.column]);
        if(tmp.row==(MAX-1) && tmp.column==(MAX-1))
            sem_post(&sem);
    }
    return NULL;
} 
/*  Nit koja vodi racuna o pocetku i kraju evolucionog ciklusa,te ispisuje tabelu celija.   */
void* function(void *arg)
{
    while(done!='q')    /*  Igra se zavrsava pritiskom na taster 'q'.   */
    {
        fp=fopen(FILENAME,"w");
        sem_post(&semaphore[0][0]); 
        sleep(SLEEP);
        sem_wait(&sem);
        fclose(fp);
        if(readMatrix())
            return NULL;
        printMatrix();
    }
    return NULL;
}
/*  Nit kojom se zavrsava igra. */
void* endThread(void *arg)
{
    while((getch())!='q')
        usleep(5000);
    done='q';
    for(int i=0; i<MAX; i++)
        for(int j=0; j<MAX; j++)
            sem_destroy(&semaphore[i][j]);
    sem_destroy(&sem);
    return NULL;
}
int main(int argc, char *argv[]) 
{ 
    pthread_t thr;
    if(argc<2)
    {
        printf("Please enter an argument.");
        return 0;
    }
    if((SLEEP=atoi(argv[1]))<=0)
    {
        printf("Argument must be positive.");
        return 0;
    }
    printf("Press 'q' to exit the game.\n");
    sleep(1);
    /*  Mora se definisati pocetno stanje tabele celija.    */
    if(readMatrix())
        return 0;
    printMatrix();
    sem_init(&sem,0,0);
    for(int i=0; i<MAX; i++)
    {
        for(int j=0; j<MAX; j++)
        {
        POSITION tmp;
        sem_init(&semaphore[i][j], 0, 0);  
        tmp.row=i;
        tmp.column=j;
        pthread_create(&thr,NULL,thread,&tmp); 
        usleep(500);
        }
    }
    pthread_create(&thr,NULL,function,0);
    pthread_create(&thr,NULL,endThread,0);
    sem_post(&sem);
    
    pthread_join(thr,NULL);
    return 0; 
}
