#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wait.h>
#include <signal.h>
#include <sys/wait.h>
#include <sched.h>

extern char ** environ;  

void print_child_status (int status, pid_t task);

int main(int argc, char *argv[])
{

	pid_t task[3];
	int status[3];
	/* Argumenti za pozivanje execve funkcije.*/
	char *argument_list[3][4] = {{"procesi", "40", "50", NULL},{"procesi", "80", "50", NULL},{"procesi", "120", "50", NULL}};
	int i = 0 ;
	int pmin = sched_get_priority_min(SCHED_FIFO);
	struct sched_param param; 
	union sigval sig;

	if (argc !=1) {
		perror ("This program does not accept command-line arguments\n");
        return -1;
	}
	
	/* Postavljanje prioriteta roditeljskog procesa na minimalnu vrijednost. */
	param.sched_priority = pmin; 
	sched_setscheduler(0, SCHED_FIFO, &param);
	
	printf("\nZA KRAJ PRITISNITE ENTER\n\n");
	
	/* Pokretanje taskova. */
	do 
	{
		task[i] = fork();
		
		if (task[i] < 0) {
			perror("Fork");
			return -1;
		}
		
		if (task[i] == 0) {
			param.sched_priority = pmin + 10 - i; 
			sched_setscheduler(0, SCHED_FIFO, &param);
			if (execve ("procesi", argument_list[i], environ) == -1) {
				perror ("execve failed");
			}
		}
		
	}while( ++i < 3 );
	
	/* Ceka ENTER za kraj programa. */
	getchar();
	
	// Parent code
	i = 0;
	while( i < 3 ){
		/* Slanje signala SIGRTMIN procesima task[i] za kraj programa. */
		sig.sival_int = i;
		if (sigqueue(task[i], SIGRTMIN,sig) == -1) {
		   printf ("kill of child failed"); 
		   return -1;
		}
		/* Cekanje da zavrsetka procesa. */
		if (waitpid (task[i], &status[i], 0) == -1) {
			perror ("waitpid failed"); 
			return -1;
		}
		i++;
	}
	/* Ispis statusa procesa. */
	for (i=0; i<3; i++){
		print_child_status(status[i],task[i]);
	}
	// Parent code

  return 0;
}

/* Funkcija koja ispisuje za kakvim statusom su zavrsi child procesi. */
void print_child_status (int status,pid_t task) {
	if (WIFEXITED (status)) {
		printf("Child[%d] exited with status %d\n", task, WEXITSTATUS (status));
	} else if (WIFSTOPPED (status)) {
		printf("Child[%d] stopped by signal %d (%d)\n", task, WSTOPSIG (status),atoi(strsignal(WSTOPSIG (status))));
	} else if (WIFSIGNALED (status)) {
		printf("Child[%d] killed by signal %d (%d)\n", task, WTERMSIG (status),atoi(strsignal(WTERMSIG (status))));
	} else {
		printf("Unknown child[%d] status\n", task);
	}
}
