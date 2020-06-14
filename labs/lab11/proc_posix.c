#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <sched.h>
#include <signal.h>
#include <string.h>

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

//Ovaj kod ce imati sva tri nova procesa, samo ce se parametri razlikovati
int start_task(void (*task)(void), int s, int ms, int max_sched_off, sigset_t* set){
	//Inicijalizacija i startovanje tajmera
	struct sigevent ev;
	memset(&ev, 0, sizeof(struct sigevent));
	ev.sigev_notify=SIGEV_SIGNAL;
	ev.sigev_signo=SIGALRM;
	
	timer_t timer;
	struct itimerspec t;
	t.it_value.tv_sec=s;
	t.it_value.tv_nsec=ms*1000000;
	t.it_interval.tv_sec=s;
	t.it_interval.tv_nsec=ms*1000000;
	if(timer_create(CLOCK_MONOTONIC, &ev, &timer)!=0 || timer_settime(timer, 0, &t, NULL)!=0){
		printf("Tajmer nije inicijalizovan!\n");
		return 1;
	}
	
	int pmax=sched_get_priority_max(SCHED_FIFO);
	struct sched_param param;
	param.sched_priority=pmax-max_sched_off; 
	sched_setscheduler(0, SCHED_FIFO, &param);
	
	int dummy;
	while(1){
		//Imamo samo 2 signala u setu koja saljemo pa ako nije jedan onda je sigurno drugi.
		sigwait(set,&dummy);
		if(dummy==SIGTERM){
			return 0;
		}
		(*task)();
	}
}

//Bice pokrenuta jos tri procesa. U slucaju da pokretanje jednog ne uspije drugi se ubijaju.
//Takodje ako unesemo karakter na stdin roditelj proces ce poslati SIGTERM djeci i tako se program moze prekinuti.
//Funkcija se poziva iz roditelja tako da on ima pravo da prekine djecu. Ako se obezbijedi da su vrijednosti pid i signala tacne ne mora se
//provjeravati povratna vrijednost sigqueue funkcije.
void terminate_parray(pid_t* parray,int len){
	//U dokumentaciji pise da se ovo koristi ako postoji handler za signal. Ne koristi se jer je SIGTERM blokiran i ceka se na njega sa
	//sigwait. Ali prilikom kompajliranja prijavljuje gresku ako se ne posalje.
	union sigval s;
	
	for(int i=0;i<len;i++){
		sigqueue(parray[i],SIGTERM,s);
	}
	return;
}

void e_xit(pid_t p, int status){
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
	
	//Djeca procesi naslijedjuju masku. A vrijednost promjenljive set se kopira prilikom stvaranje novih procesa.
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGALRM);
	sigaddset(&set,SIGTERM);
	sigprocmask(SIG_BLOCK,&set,NULL);
	
	p[0]=getpid();
	
	//task3
	p[1]=start_new_p();
	if(p[1]<0){
		return -1;
	}
	if(p[1]==0){
		return start_task(task3, 0, 120, 30,&set);
	}
	
	//task2
	p[2]=start_new_p();
	if(p[2]<0){
		terminate_parray(p+1,1);
		return -1;
	}
	if(p[2]==0){
		return start_task(task2, 0, 80, 20,&set);
	}
	
	//task1
	p[3]=start_new_p();
	if(p[3]<0){
		terminate_parray(p+1,2);
		return -1;
	}
	if(p[3]==0){
		return start_task(task1, 0, 40, 10,&set);
	}
	
	int pmax=sched_get_priority_max(SCHED_FIFO);
	struct sched_param param;
	param.sched_priority=pmax; 
	sched_setscheduler(0, SCHED_FIFO, &param);
	
	fgetc(stdin);
	
	printf("\n");
	int status;
	terminate_parray(p+1,3);
	waitpid(p[3],&status,0);
	e_xit(p[3],status);
	waitpid(p[2],&status,0);
	e_xit(p[2],status);
	waitpid(p[1],&status,0);
	e_xit(p[1],status);
	
	return 0;
}

