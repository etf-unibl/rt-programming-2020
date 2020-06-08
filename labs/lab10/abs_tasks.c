#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>

static int control=0;

typedef struct in{
	int x;
	int y;
	int info;
	//vrijeme u ms
	int time;
}INFO;

static inline void add_time(struct timespec* t,uint64_t ms){
	ms=ms*1000000;
	ms=ms+t->tv_nsec;
	t->tv_sec=t->tv_sec+ms/1000000000;
	t->tv_nsec=ms%1000000000;
	return;
}

//Periodični zadaci koje je trebalo realizovati se dobijaju proslijedjivanjem parametara funkciji periodic_task.
//Po logici videa petlja je ugnježdena, ali u kodu u dostavljenom fajlu nije. Ovdje je realizovana kako ugnježdena.
void* periodic_task(void* arg){
	INFO* a=(INFO*)arg;
	
	//Pocetno vrijeme
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	add_time(&t,a->time);
	
	int i,j;
	while(control==0){
		//Cekanje
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &t, NULL);
		add_time(&t,a->time);
		for (i=0; i<a->x; i++) {
			for (j=0; j<a->y; j++){
				printf("%d",a->info);
				fflush(stdout);
			}
		}
	}
	return NULL;
}

int main(){
	pthread_t t1, t2, l;
	INFO i1,i2;
	i1.x=4;
	i1.y=1000;
	i1.info=1;
	i1.time=60;
	i2.x=6;
	i2.y=10000;
	i2.info=2;
	i2.time=80;
	//U slucaju da ne uspije inicijalizacija jedne od niti koja izvrsava periodicni zadatak druga ce prestati sa radom.
	control=control+pthread_create(&t1,NULL,periodic_task,(void*)(&i1));
	control=control+pthread_create(&t2,NULL,periodic_task,(void*)(&i2));
	
	fgetc(stdin);
	control=1;
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	
	
	printf("\n");
	
	return 0;
}

