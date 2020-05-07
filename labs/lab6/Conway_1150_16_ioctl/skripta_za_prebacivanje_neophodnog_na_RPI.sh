#!/bin/bash

TARGETHOST="pi@localhost"
TARGETPATH="/home/pi/"
DIR="Conway_ioctl"

# Funkcija za Kreiranje direktorijuma na RPI!
ensure_dir() {
	# create dir
    echo "Creating remote directory: $TARGETPATH$1"
    ssh -p 5022 $TARGETHOST "mkdir -p ${TARGETPATH}$1"
}

#Kreiranje direktorijuma na RPI!
ensure_dir ${DIR}
# Kopiranje neophodnog!
cd bin
echo "Moving files to remote directory: $TARGETPATH${DIR}"
scp -P 5022 conway_rpi mapa.txt ../modul/ioctl_modul.ko pi@localhost:${TARGETPATH}${DIR}

