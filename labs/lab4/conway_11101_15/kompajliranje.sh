mkdir bin
mkdir obj
make CC=arm-linux-gnueabihf-gcc
cd src
gcc -pthread -o  conway_gcc conway.c
mv conway_gcc ../bin
