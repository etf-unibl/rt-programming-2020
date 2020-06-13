Iz foldera u kojem se nalazi:
    Kompajliranje: arm-linux-gnueabihf-gcc procesi.c -o procesi
    Prebacivanje na qemu: sudo scp -P 5022 procesi pi@localhost:procesi
U qemu: ./procesi

