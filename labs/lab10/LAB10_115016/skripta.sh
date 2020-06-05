#!/bin/bash

TARGETHOST="pi@localhost"
TARGETPATH="/home/pi/"
PORT="5022"
DIR="Lab10_Dejan"

# Funkcija za Kreiranje direktorijuma na RPI!
ensure_dir() {
	# create dir
    echo "Creating remote directory: $TARGETPATH$1"
    ssh -p $PORT $TARGETHOST "mkdir -p ${TARGETPATH}$1"
}

clear # Brisanje!
# Kroskompajliranje!
arm-linux-gnueabihf-gcc periodic_tasks.c -std=gnu99 -Wall -lrt -o periodic_tasks_arm -lpthread
if test $? -eq 0 #Ukoliko je uspjesno kompajlirano, prebacimo na QEMU rpi!
then
	#Kreiranje direktorijuma na RPI!
	ensure_dir ${DIR}
	echo "Moving files to remote directory: $TARGETPATH${DIR}"
	scp -P $PORT periodic_tasks_arm $TARGETHOST:${TARGETPATH}${DIR}
fi

# GCC kompajliranje, za izvrsavanje na hostu!
gcc -Wall periodic_tasks.c -lrt -o periodic_tasks_gcc -lpthread


