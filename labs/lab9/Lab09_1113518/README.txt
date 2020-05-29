Nisam uspio namjestiti da radi kako treba, uspio sam napraviti tredove i namjestio sam mutex kako je objasnjeno.
H mi je prioriteta 99 tj makismalnog dozvoljenog, L je 1 tj minimalno dozvoljenog a M je aritmeticka sredina ta dva tj 49.
Kompajlirao samm na linuxu pa onda prebacivao na QEMU sa sleddecim komandama:
    arm-linux-gnueabihf-gcc zadaca.c -g3 -o zadaca -lpthread -lrt
    arm-linux-gnueabihf-strip zadaca -s -o zadaca
    sudo scp -P 5022 zadaca pi@localhost:zadaca
Na QEMU pokreto sa:
    sudo ./zadaca
