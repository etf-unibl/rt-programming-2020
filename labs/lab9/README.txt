PC: scp -P 5022 thread_template.c pi@localhost:

PI(QEMU): gcc thread_template.c -o execute -lpthread -lrt -Wall 
PI(QEMU): sudo ./execute    
