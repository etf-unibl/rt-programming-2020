Nisam dobio poruku o tome kojom metodom da uradim zadaću pa sam čekao, jer sam mislio da svi čekaju. Poslije sam shvatio da su svi dobili i tad nisam bio siguran da li
ćete vidjeti poruku na vrijeme pa sam predao za sva tri slučaja.

Prvi slučaj signal_threads.c
arm-linux-gnueabihf-gcc signal_threads.c -pthread -std=gnu99 -Wall -o signal_threads
gcc signal_threads.c -pthread -std=gnu99 -Wall -o signal_threads

Drugi slučaj rt_posix_timer_tasks1.c
arm-linux-gnueabihf-gcc rt_posix_timer_tasks1.c -pthread -lrt -std=gnu99 -Wall -o rt_posix_timer_tasks1
gcc rt_posix_timer_tasks1.c -pthread -lrt -std=gnu99 -Wall -o rt_posix_timer_tasks1

Treći slučaj abs_tasks.c
arm-linux-gnueabihf-gcc -pthread -o abs_tasks abs_tasks.c
gcc -pthread -o abs_tasks abs_tasks.c
