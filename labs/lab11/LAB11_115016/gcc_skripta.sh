# Kompajliranje + Pokretanje!
clear
gcc zadatak.c -o zadatak_gcc -lpthread -lrt -Wall
if test $? -eq 0 #Ukoliko je uspjesno kompajlirano,izvrsavamo!
then
	sudo ./zadatak_gcc
fi
