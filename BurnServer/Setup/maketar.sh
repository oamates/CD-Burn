#!/bin/sh

##################################################
VERSION=1.0.0.10
##################################################
cd ../
make  clean
make

cp		./AuthChnsys.exe			/mnt/hgfs/BurnServer/Setup/BurnServerSetup/BurnServer/
cp		./BurnServer.out			/mnt/hgfs/BurnServer/Setup/BurnServerSetup/BurnServer/
cp		./Get_CDRom_Dev_Info.sh		/mnt/hgfs/BurnServer/Setup/BurnServerSetup/BurnServer/
cp -r	./PlayBack					/mnt/hgfs/BurnServer/Setup/BurnServerSetup/BurnServer/
cp -r	./config					/mnt/hgfs/BurnServer/Setup/BurnServerSetup/BurnServer/


##################################################
cd /mnt/hgfs/BurnServer/Setup/
tar -cvf BurnServer$VERSION.tar.gz BurnServerSetup


