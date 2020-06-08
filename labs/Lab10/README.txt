Kompajliranje:
    arm-linux-gnueabihf-gcc periodic_task_abs_time.c -o periodic_task_abs_time -lpthread -lrt
Prebacivanje na QEMU:
    sudo scp -P 5022 periodic_task_abs_time pi@localhost:periodic_task_abs_time
Pokretanje na QEMU:
    ./periodic_task_abs_time
