    U ~/.bashrc namjesteno sljedece:
    export TOOL_PATH=/home/marko/tools/arm-bcm2708/arm-linux-gnueabihf/bin
    export PATH=$PATH:$TOOL_PATH
    export ARCH=arm
    export CROSS_COMPILE=arm-linux-gnueabihf-
    export KERNEL=kernel7
    export LINUX_SRC=~/linux
   
    U lab6 direktorijumu: 
                sudo make all (ako zelimo igru zivota za nas os)
                sudo scp -P 5022 matrica.txt pi@localhost:matrica.txt
    U src folderu:
            arm-linux-gnueabihf-gcc conway.c -g3 -o conwaypi -lpthread -lrt
            sudo scp -P 5022 conwaypi pi@localhost:conwaypi

    U mod folderu:   
           sudo make all 
           sudo scp -P 5022 proces.ko pi@localhost:proces.ko
 
 Na QEMU, unjeti sljedece komande: 
 
  sudo insmod proces.ko 
  ./conwaypi matrica.txt 4

    
