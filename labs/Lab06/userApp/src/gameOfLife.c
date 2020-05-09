#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <sys/stat.h>
#include <fcntl.h>

#define MAX 10 

char getch(void);
char buffer[MAX*MAX];
int SLEEP=0;
int matrix[MAX][MAX];
volatile char done=0x0;
sem_t semaphore[MAX][MAX];

int file_desc;
unsigned int brojZivihCelija=0;
unsigned int brojRodjenihCelija=0;
unsigned int brojUmrlihCelija=0;
unsigned int brojGeneracija=0;

/*  Struktura koja opisuje poziciju celije u matrici.   */
typedef struct position
{
     int row;
     int column;
} POSITION;
/*  Unija converter pomaze pri upisu podataka tipa unsigned int u niz charova.   */
union converter
{
	unsigned int source;
	char tgt[sizeof(int)];
};
union converter conv;

/*  Ispis na prazan ekran.  */
void clear_screen(void)
{
	const char* CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
	write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}
/*  Citanje stanja celija iz kernela. Celije su opisane brojevima 1(ziva) i 0(mrtva).  */
int readMatrix()
{
    char c;
    int x;
    if((read(file_desc, buffer, MAX*MAX))!=(MAX*MAX))
        return -1;
    else
    {
        for(int i=0; i<MAX; i++)
        {
            for(int j=0; j<MAX; j++)
            {
                c=buffer[i*MAX+j];
                x=c-'0';
                if(x<0 || x>1)
                        return -1;
                matrix[i][j]=x;
            }
        }
    }
    return 0;
}
/*  Citanje statistike iz kernela.  */
int procitajStatistiku()
{
    if((read(file_desc, buffer, 16))!=16)
        return -1;
    else
    {
        for(int i=0; i<16; i++)
        {
            conv.tgt[i%4]=buffer[i];
            switch(i)
            {
                case 3  :   brojZivihCelija=conv.source;
                    break;
                case 7  :   brojRodjenihCelija=conv.source;
                    break;
                case 11  :   brojUmrlihCelija=conv.source;
                    break;
                case 15 :   brojGeneracija=conv.source;
            }
        }
    }
    return 0;
}
/*  Ispis matrice koja predstavlja tabelu sa celijama na standardni izlaz i statistike.  */
void print()
{
    clear_screen();
    for(int i=0; i<MAX; i++)
    {
        for(int j=0; j<MAX; j++)
        printf("%d ", matrix[i][j]);
        printf("\n");
    }
    printf("\nBroj zivih celija: %u \n", brojZivihCelija);
    printf("Broj rodjenih celija: %u \n", brojRodjenihCelija);
    printf("Broj umrlih celija: %u \n", brojUmrlihCelija);
    printf("Broj generacija: %u \n", brojGeneracija); 
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
void upisi(int k, int p)    //Upisuje stanje jedne celije na odgovarajuce mesto u bufferu kernela.
{
    buffer[0]='0';  //znak da se radi o upisu stanja celije
    conv.source=k;  //pozicija upisa
    buffer[1]=conv.tgt[0]; 
    buffer[2]=conv.tgt[1];
    buffer[3]=conv.tgt[2];
    buffer[4]=conv.tgt[3];  
    char c=p+'0';
    buffer[5]=c;
    write(file_desc, buffer, 6);
}
/*  Upis statistike u buffer kernela.   */
void upisiStatistiku()
{
    for(int i=0; i<16; i++)     //16 bajtova-dovoljno za 4 podatka tipa unsigned int
    {   
        switch(i)
        {
            case 0  :   conv.source=brojZivihCelija;
                break;
            case 4  :   conv.source=brojRodjenihCelija;
                break;
            case 8  :   conv.source=brojUmrlihCelija;
                break;
            case 12 :   conv.source=brojGeneracija;
        }
        buffer[i]=conv.tgt[i%4];
    }
    write(file_desc, buffer, 16);
}
/*  Funkcija koja se poziva posle obavljenog ciklusa evolucije.Poziva funkciju za ispis.     */
void function()
{
        sleep(SLEEP);
        brojGeneracija++;
        upisiStatistiku();
        if(readMatrix())
            printf("Greska u citanju matrice.\n");
        if(procitajStatistiku())
            printf("Greska u citanju statistike.\n");
        print();
        brojZivihCelija=0;
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
                upisi(tmp.row*MAX+tmp.column,1);
                brojZivihCelija++;
            }
        else if (matrix[tmp.row][tmp.column]==0 && n==3)
            {
                upisi(tmp.row*MAX+tmp.column,1);
                brojZivihCelija++;
                brojRodjenihCelija++;
            }
        else if(matrix[tmp.row][tmp.column]==1)
            {
                upisi(tmp.row*MAX+tmp.column,0);
                brojUmrlihCelija++;
            }
        if(tmp.row==(MAX-1) && tmp.column==(MAX-1))
            function();
        tmp=nextElement(tmp.row,tmp.column);
        sem_post(&semaphore[tmp.row][tmp.column]);
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
    if((file_desc = open("/proc/stat_proc", O_RDWR))<0)
        printf("/proc/stat_proc not open. \n");
    printf("Press 'q' to exit the game.\n");
    sleep(1);
    if(readMatrix())
    {
        printf("Greska u citanju matrice. \n");
        return 0;
    }
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
    pthread_create(&thr,NULL,endThread,0);
    sem_post(&semaphore[0][0]);
    
    pthread_join(thr,NULL);
    return 0; 
}
