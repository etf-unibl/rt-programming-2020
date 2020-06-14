Kompajliranje:
    arm-linux-gnueabihf-gcc fork.c -o fork
Prebacivanje na Qemu:
    sudo scp -P 5022 fork pi@localhost:fork
Pokretanje na Qemu:
    ./fork
