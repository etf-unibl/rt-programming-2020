Kompajliranje: 
    arm-linux-gnueabihf-gcc periodic_task_abs_time.c -std=gnu99 -Wall -o periodic_task_abs_time
Prebacivanje na Qemu:
    sudo scp -P 5022 periodic_task_abs_time pi@localhost:periodic_task_abs_time

