#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <termios.h>

#define DIMENSION 10
#define DIMARR 100


 




//pomocna struktura pomocu koje rjesavam proble sa prioritetima
typedef struct{
    
    int pos_x;
    int pos_y;
    int prio;
    
}prio_el;

/*strukutra jedne celije(polja matrice)--koristim je u tredovsku f-ju za obradu zivih i mrtvih celija*/
typedef struct{
 
    int x_pos;
    int y_pos;
    bool is_alive;
}CELL;



//Globalne
static pthread_t thread_matrix[DIMENSION][DIMENSION];
static CELL new_matrix[DIMENSION][DIMENSION];
static CELL old_matrix[DIMENSION][DIMENSION];

static pthread_t ending; 
static pthread_mutex_t critical_section_a;
static sem_t semaphore;
 
static int live_cells=0;
static int cells_died=0;
static int generation=0;
static int cells_born=0;
static bool not_done=true;

prio_el mat[DIMENSION][DIMENSION];
prio_el exec_ord[DIMARR];



//Deklaracija funkcija

//void loading_pattern();//--->ucitavanje sablona simulacije-->matrica 10x10 txt format
int neighbours();//-->racuna broj komsija
int yadd(int,int);//pomocne funkcije za racunanje koordinata komsija 
int xadd(int,int);//pomocne funkcije za racunanje koordinata komsija
int thread_matrix_init();
void print();//funkcija za ispis na konzolu
void cell_matrix_init(char c);//inicijalizacija celija
void new_to_old();///nova generacija postaje stara

void* cells_evo(void*);//tredovska f-ja
void* end_function(void*);
void sort(prio_el[]);//funkcija za sortiranje
void execution_order();//funkcija koja ce da odredi redoslijed izvrsavanje tredova na osnovu prioriteta

char getch(void);



int main(){
    
    sem_init(&semaphore,0,0);
    
    unsigned int sleep_time;
    char c;
    printf("Izaberite vas sablon-genraciju za razvoj.\n");
    printf("Za zeljeni sablon stisnite dato slovo:\nGlider->g\nOscilator->o\nBeehive->b\nf-Pentomino->f\n");
    scanf("%c",&c);
    
    printf("\n");
    
    printf("Unesite trajanje spavavja glavne niti(unos u sekundama):");
    scanf("%d",&sleep_time);
    
    
    
    sem_post(&semaphore);
    
    execution_order();//odredimo prioritete izvrsavanja
    cell_matrix_init(c);//inicijalizujemo prvu generaciju
    pthread_create(&ending,NULL,end_function,0);
    print();//ispisemo prvu generaciju
    sleep(sleep_time);
   
   
    while(not_done){
        sem_wait(&semaphore);
        thread_matrix_init();//pokrecemo tredove
        print();//ispisujemo svaku sledecu generaciju
        sleep(sleep_time);
        
        
    }
   
   sem_destroy(&semaphore);
   pthread_join(ending,NULL);
   return 0;

 
}





void *end_function(void*arg)
{
    if(getch()=='q')
        not_done=false;
        
    return NULL;
}




void execution_order() {

	
	int prvi = 1;
	for (int i = 0; i < DIMENSION; i++) {
		for (int j = 0; j < DIMENSION; j++) {
//cuvamo koordinate i prioritet u strukturi pomocu prio_el u formi matrice
			mat[i][j].pos_x = i;
			mat[i][j].pos_y = j;
			mat[i][j].prio = prvi + j;

		}
		prvi += 2;
	}

//prebacujem sve elemente iz 2D u 1D niz i sortiram na osnovu prioriteta izvrsavanja
	for (int i = 0; i < DIMENSION; i++) {
		for (int j = 0; j < DIMENSION; j++) {
			exec_ord[i * DIMENSION + j] = mat[i][j];
		}
	}
	sort(exec_ord);
}


void  sort(prio_el ar[DIMARR]) {
	for (int i = 0; i < DIMARR; i++)
	{
		for (int j = 0; j < DIMARR - 1; j++)
		{
			if (ar[j].prio > ar[j + 1].prio)
			{
				prio_el temp = ar[j];
				ar[j] = ar[j + 1];
				ar[j + 1] = temp;
			}
		}
	}
}





//funkcija koja ucitava sablon iz txt file i inicijalizuje matricu tipa CELL 

void cell_matrix_init(char c){
    FILE *file;
    char tmp_mat[DIMENSION][DIMENSION];
    switch(c){
    
    case 'g':      
        file=fopen("glider.txt","r");
        break;
    case 'b':
        file=fopen("beehive.txt","r");
        break;
    case 'o':
        file=fopen("oscilator.txt","r");
        break;
    case 'f':
        file=fopen("f_pentomino.txt","r");
        break;
    default:
        printf("Zeljena opcija ne postoji molimo vas da ponovo pokrenete program i unesite\n   jednu od ponudjenih opcija.\n");
    
    }
	if(file==NULL)
	{
		printf("Unable to open file\n");
	}
    
    for(int i = 0; i <DIMENSION; i++){
      for(int j = 0; j <DIMENSION; j++) 
      {

       fscanf(file,"%c\n", &tmp_mat[i][j]); 
        
      
       new_matrix[i][j].x_pos=i;
       new_matrix[i][j].y_pos=j;
       if(tmp_mat[i][j]=='1'){
         new_matrix[i][j].is_alive=true;
       live_cells++;
       
    }
       else{
        new_matrix[i][j].is_alive=false;
        
        
      }
    }

  }
 
  new_to_old();
  generation++;
  fclose(file);
}
    
    
//funkcija pomocu koje nova generacija postaje stara    
void new_to_old(){
    
    for(int i = 0; i <DIMENSION; i++){
      for(int j = 0; j <DIMENSION; j++){
          old_matrix[i][j].x_pos=i;
          old_matrix[i][j].y_pos=j;
          old_matrix[i][j].is_alive=new_matrix[i][j].is_alive;
          } 
    }
}    

//tredovska f-ja
void *cells_evo(void *param){
    
    CELL curr_cell=*(CELL*)param;
    bool alive=curr_cell.is_alive;
    
    
    pthread_mutex_lock(&critical_section_a);
    
    
    int live_neighbours=0;
    live_neighbours=neighbours(curr_cell.x_pos,curr_cell.y_pos);//provjera zivih komsija
    //evolucija po pravilima game of life
    if(alive){
    if(live_neighbours==2)new_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive=
        old_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive;
   
    if(live_neighbours<2)
     {
        live_cells--;
        cells_died++;
        
        new_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive=false;
     }
    if(live_neighbours>3)
      {
        
        live_cells--;
        cells_died++;
        
        new_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive=false;
      }
    }
    else
    {
        if(live_neighbours==3)
        {
            live_cells++;
            cells_born++;

            new_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive=true;
        }
    }
    
    
  
  
    sem_post(&semaphore);
    pthread_mutex_unlock(&critical_section_a);
    
    return NULL;
}
        
        

    


int xadd(int i,int a){
    i+=a;
    while(i<0)i +=DIMENSION;
    while(i>=DIMENSION)i-=DIMENSION;
    return i;
}


int yadd(int i,int a){
    i+=a;
    while(i<0)i+=DIMENSION;
    while(i>=DIMENSION)i-=DIMENSION;
    return i;
    
}


 
int neighbours(int i,int j){
    
    
    int count=0;
    
    for(int k=-1;k<=1;k++){
        for(int l=-1;l<=1;l++)
            if(k||l)
                if(old_matrix[xadd(i,k)][yadd(j,l)].is_alive)
                    count++;
    }
                    
     return count;
    
    
    
}    
    
  //ovo f-jom inicijalizujem i pokrecem sve tredove  
int thread_matrix_init(){
     
    
     
     pthread_mutex_init(&critical_section_a,NULL);
     
     //ovdje pokrecemo i sinhronizujemo sve tredove po prioritetima 
     for(int i=0;i<DIMARR;i++){
        
          if(pthread_create(&thread_matrix[exec_ord[i].pos_x][exec_ord[i].pos_y],NULL,cells_evo,
             (void*)&new_matrix[exec_ord[i].pos_x][exec_ord[i].pos_y]))
             return -1;
     }
     for(int c=0;c<DIMARR;c++){
         
         pthread_join(thread_matrix[exec_ord[c].pos_x][exec_ord[c].pos_y],NULL);
         
     }
     
     pthread_mutex_destroy(&critical_section_a);
     
     
     new_to_old();
     generation++;

     
     
     return 0;

 }



void print (){
    int i,j;
    puts("\033[H\033[J");
    printf("PRESS 'q' for EXIT!\n");
    for(i=0;i<DIMENSION;i++){
        for(j=0;j<DIMENSION;j++){
            printf("%s",old_matrix[i][j].is_alive ? "■ ":"□ ");
        }
        printf("\n");
    }
    printf("statistics:\n");
    printf("Live cells: %d\n",live_cells);
    printf("Died cells: %d\n",cells_died);
    printf("Born cells: %d\n",cells_born);
    printf("Generations: %d\n",generation);

    cells_born=0;
    cells_died=0;


}

char getch(void)
{
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
   // printf("%c\n", buf);
    return buf;
 }

