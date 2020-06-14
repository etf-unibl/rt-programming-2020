proc_signal.c
arm-linux-gnueabihf-gcc proc_signal.c -std=gnu99 -Wall -o proc_signal
gcc proc_signal.c -std=gnu99 -Wall -o proc_signal

proc_posix.c
arm-linux-gnueabihf-gcc proc_posix.c -lrt -std=gnu99 -Wall -o proc_posix
gcc proc_posix.c -lrt -std=gnu99 -Wall -o proc_posix

proc_abs.c
arm-linux-gnueabihf-gcc -o proc_abs proc_abs.c
gcc -pthread -o abs_tasks abs_tasks.c
