echo "making folders"
sudo mkdir  bin
sudo chmod 777 bin 
sudo mkdir  obj
sudo chmod 777 obj
echo "PI"  
make CC=arm-linux-gnueabihf-gcc EXEC_NAME=cnwy_pi
cd obj
sudo mv conway.o conwaypi.o
cd ..
echo "--------------------------"
echo " PC "
make all
echo "#COMPLETED SCRIPT!"
