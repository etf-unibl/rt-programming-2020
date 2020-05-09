mkdir -p bin
mkdir -p obj
make CC=arm-linux-gnueabihf-gcc
cd src
gcc -pthread -o  conway_gcc conway.c
mv conway_gcc ../bin
