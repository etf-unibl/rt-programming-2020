mkdir -p lab6_bin
cp modul/sistem.ko lab6_bin
cp conway_11101_15/bin/conway_rpi lab6_bin
cp conway_11101_15/bin/map.txt lab6_bin
scp -pr -P 5022 lab6_bin  pi@localhost:/home/pi/
