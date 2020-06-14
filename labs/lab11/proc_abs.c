#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

//40 milisekundi
void task1(void)
{
  int i,j;
 
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
}

//80 milisekundi
void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
}

//120 milisekundi
void task3(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<100000; j++) ;
    printf("3");
    fflush(stdout);
  }
}

//Da ne bi stalno ponavljali jedan te isti kod.
pid_t start_new_p(){
	pid_t p=fork();
	if(p<0){
		printf("Greska pri pokretanju procesa!\n");
	}
	return p;
}

static inline void add_time(struct timespec* t,uint64_t ms){
	ms=ms*1000000;
	ms=ms+t->tv_nsec;
	t->tv_sec=t->tv_sec+ms/1000000000;
	t->tv_nsec=ms%1000000000;
	return;
}

//Ovaj kod ce imati sva tri nova procesa, samo ce se parametri razlikovati
void start_task(void (*task)(void), int ms, int max_sched_off){
	//Prioritet
	int pmax=sched_get_priority_max(SCHED_FIFO);
	struct sched_param param;
	param.sched_priority=pmax- max_sched_off; 
	sched_setscheduler(0, SCHED_FIFO, &param);
	
	//Vrijeme
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	add_time(&t,ms);
	
	while(1){
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &t, NULL);
		add_time(&t,ms);
		(*task)();
	}
}

//Bice pokrenuta jos tri procesa. U slucaju da pokretanje jednog ne uspije drugi se ubijaju.
//Takodje ako unesemo karakter na stdin roditelj proces ce poslati SIGTERM djeci i tako se program moze prekinuti.
//Funkcija se poziva iz roditelja tako da on ima pravo da prekine djecu. Ako se obezbijedi da su vrijednosti pid i signala tacne ne mora se
//provjeravati povratna vrijednost sigqueue funkcije.
void terminate_parray(pid_t* parray,int len){
	union sigval s;
	
	for(int i=0;i<len;i++){
		sigqueue(parray[i],SIGTERM,s);
	}
	return;
}

void e_xit(pid_t p, int status){
	//Prvi uslov u ovom slucaju se nece nikad ispuniti jer djeca nikad sama ne prekinu sa exit tj. return.
	//Treci uslov ce uvijek biti ispunjen kada se izlazi iz programa unosom karaktera na stdinjer se salje SIGTERM i ubija procese.
	//Drugi se moze ispuniti samo ako neki drugi proces posalje signal za zaustavljanje.
	if(WIFEXITED (status)){
		printf("Child exited with status %d\n", WEXITSTATUS (status));
	}
	else if(WIFSTOPPED (status)){
		printf("Child stopped by signal %d (%s)\n", WSTOPSIG (status), strsignal(WSTOPSIG (status)));
	}
	else if(WIFSIGNALED (status)) {
		printf("Child killed by signal %d (%s)\n", WTERMSIG (status), strsignal(WTERMSIG (status)));
	}
	else{
		printf("Unknown child status\n");
	}
}

int main(){
	//Ovdje se smjestaju svi pid od djece tako da se kasnije ti preocesi mogu prekinuti.
	pid_t p[4];
	p[0]=getpid();
	
	//task3
	p[1]=start_new_p();
	if(p[1]<0){
		return -1;
	}
	if(p[1]==0){
		start_task(task3, 120, 30);
	}
	
	//task2
	p[2]=start_new_p();
	if(p[2]<0){
		terminate_parray(p+1,1);
		return -1;
	}
	if(p[2]==0){
		start_task(task2, 80, 20);
	}
	
	//task1
	p[3]=start_new_p();
	if(p[3]<0){
		terminate_parray(p+1,2);
		return -1;
	}
	if(p[3]==0){
		start_task(task1, 40, 10);
	}
	
	int pmax=sched_get_priority_max(SCHED_FIFO);
	struct sched_param param;
	param.sched_priority=pmax; 
	sched_setscheduler(0, SCHED_FIFO, &param);
	
	fgetc(stdin);
	
	int status;
	terminate_parray(p+1,3);
	printf("\n");
	waitpid(p[3],&status,0);
	e_xit(p[3],status);
	waitpid(p[2],&status,0);
	e_xit(p[2],status);
	waitpid(p[1],&status,0);
	e_xit(p[1],status);
	
	return 0;
}
