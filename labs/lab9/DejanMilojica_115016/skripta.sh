#!/bin/bash

TARGETHOST="pi@localhost"
TARGETPATH="/home/pi/"
PORT="5022"
DIR="Dejan_115016"

# Funkcija za Kreiranje direktorijuma na RPI!
ensure_dir() {
	# create dir
    echo "Creating remote directory: $TARGETPATH$1"
    ssh -p $PORT $TARGETHOST "mkdir -p ${TARGETPATH}$1"
}

clear # Brisanje!
# Kroskompajliranje!
arm-linux-gnueabihf-gcc zadatak.c -o zadatak -lpthread -lrt -Wall
if test $? -eq 0 #Ukoliko je uspjesno kompajlirano, prebacimo na QEMU rpi!
then
	#Kreiranje direktorijuma na RPI!
	ensure_dir ${DIR}
	echo "Moving files to remote directory: $TARGETPATH${DIR}"
	scp -P $PORT zadatak $TARGETHOST:${TARGETPATH}${DIR}
fi

