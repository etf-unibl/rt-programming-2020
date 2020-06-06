Kompajliranje:
    arm-linux-gnueabihf-gcc periodic_tasks.c -std=gnu99 -lpthread -lrt -Wall -o periodic_tasks
Kopiranje na QEMU:
    sudo scp -P 5022 periodic_tasks pi@localhost:periodic_tasks
Pokretanje na QEMU:
    ./periodic_tasks
