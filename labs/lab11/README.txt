PC: scp -P 5022 fork.c pi@localhost:fork.c

PI(QEMU): gcc fork.c -o fork -std=gnu99  -Wall 
PI(QEMU): sudo ./fork   
