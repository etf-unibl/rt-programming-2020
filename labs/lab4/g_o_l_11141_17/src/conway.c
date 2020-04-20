#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

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

static pthread_mutex_t critical_section_a;

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
void sort(prio_el[]);//funkcija za sortiranje
void execution_order();//funkcija koja ce da odredi redoslijed izvrsavanje tredova na osnovu prioriteta





int main(){
    
    char c;
    printf("Izaberite vas sablon-genraciju za razvoj.\n");
    printf("Za zeljeni sablon stisnite dato slovo:\nGlider->g\nOscilator->o\nBeehive->b\nf-Pentomino->f\n");
    scanf("%c",&c);
    
    printf("\n");
    
    
    
    execution_order();//odredimo prioritete izvrsavanja
    cell_matrix_init(c);//inicijalizujemo prvu generaciju
    print();//ispisemo prvu generaciju
    sleep(2);
    while(1){
        
        thread_matrix_init();//pokrecemo tredove
        print();//ispisujemo svaku sledecu generaciju
        sleep(2);
        
        
    }	
   
   return 0;

 
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
       if(tmp_mat[i][j]=='1')
        new_matrix[i][j].is_alive=true;
       else   
        new_matrix[i][j].is_alive=false;
      }

  }
 
  new_to_old();
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
    pthread_mutex_lock(&critical_section_a);
    int live_neighbours=0;
    live_neighbours=neighbours(curr_cell.x_pos,curr_cell.y_pos);//provjera zivih komsija
    //evolucija po pravilima game of life
    if(live_neighbours==2)new_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive=
        old_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive;
    if(live_neighbours==3)new_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive=true;
    if(live_neighbours<2)new_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive=false;
    if(live_neighbours>3)new_matrix[curr_cell.x_pos][curr_cell.y_pos].is_alive=false;
    
    pthread_mutex_unlock(&critical_section_a);
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
     
     
     
     
     return 0;

 }



void print (){
    int i,j;
    puts("\033[H\033[J");
    for(i=0;i<DIMENSION;i++){
        for(j=0;j<DIMENSION;j++){
            printf("%s",old_matrix[i][j].is_alive ? "■ ":"□ ");
        }
        printf("\n");
    }

}

