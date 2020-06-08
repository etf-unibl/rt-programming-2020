Kompajliranje za QEMU:
    arm-linux-gnueabihf-gcc -Wall 11131_17_periodic_task_posix_timer.c -lrt -lpthread -o 11131_17_periodic_task_posix_timer

Kompajliranje za linux:
gcc -Wall 11131_17_periodic_task_posix_timer.c -lrt -lpthread -o 11131_17_periodic_task_posix_timer

Prebacivanje na QEMU:
    sudo scp -P 5022 11131_17_periodic_task_posix_timer pi@localhost:11131_17_periodic_task_posix_timer
Pokretanje na QEMU:
    ./11131_17_periodic_task_posix_timer
