Kompajliranje za QEMU:
    arm-linux-gnueabihf-gcc -Wall lab11_procesi.c -lrt -lpthread -o lab11_procesi

Kompajliranje za linux:
gcc -Wall lab11_procesi.c -lrt -lpthread -o lab11_procesi

Prebacivanje na QEMU:
    sudo scp -P 5022 lab11_procesi pi@localhost:lab11_procesi
Pokretanje na QEMU:
    ./lab11_procesi
