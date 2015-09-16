#!/bin/sh

##################################################
HOME_DIR=/home
BURNSERVER_DST_DIR=/home/BurnServer
BURNSERVER_SRC_DIR=./BurnServer
CONFIG_SRC_DIR=./BurnServer/config
CONFIG_DST_DIR=/home/BurnServer/config
START_SCRIPT_SRC_PATH=./S57BURNSERVER
START_SCRIPT_DST_PATH=/etc/rc.d/rc3.d/S57BURNSERVER
START_SCRIPT_DST_PATH_1=/etc/rc.d/rc5.d/S57BURNSERVER


##################################################
echo "start to setup BurnServer!"


##################################################
# kill BurnServer
echo "kill current running BurnServer!"
pkill -9 BurnServer


####################################################################################################
if [ -d $BURNSERVER_DST_DIR ]; then
	##################################################
	# only update the program
	##################################################
	# del origin program
	echo "del current version!"
	
	rm -f	$BURNSERVER_DST_DIR/AuthChnsys.exe
	rm -f	$BURNSERVER_DST_DIR/BurnServer.out
	rm -f	$BURNSERVER_DST_DIR/Get_CDRom_Dev_Info.sh
	rm -rf	$BURNSERVER_DST_DIR/PlayBack
	rm -rf	$BURNSERVER_DST_DIR/config
	
	##################################################
	# copy the latest program
	echo "copy latest version!"
	
	cp $BURNSERVER_SRC_DIR/AuthChnsys.exe			$BURNSERVER_DST_DIR/AuthChnsys.exe
	cp $BURNSERVER_SRC_DIR/BurnServer.out			$BURNSERVER_DST_DIR/BurnServer.out
	cp $BURNSERVER_SRC_DIR/Get_CDRom_Dev_Info.sh	$BURNSERVER_DST_DIR/Get_CDRom_Dev_Info.sh
	cp -r $BURNSERVER_SRC_DIR/PlayBack				$BURNSERVER_DST_DIR/PlayBack
	cp -r $BURNSERVER_SRC_DIR/config				$BURNSERVER_DST_DIR/config
else
	##################################################
	cp -r $BURNSERVER_SRC_DIR $HOME_DIR/
fi
####################################################################################################

##################################################
# if have config folder, do not copy config, else copy config
if [ ! -d $CONFIG_DST_DIR ]; then
	echo "setup first time ,copy config folder!"
	
	cp -r $CONFIG_SRC_DIR $CONFIG_DST_DIR
else
	echo "already have config!"
fi


##################################################
# copy start script to /etc/rc.d/rc3.d/
echo "copy start script S57BURNSERVER!" 
if [ -f $START_SCRIPT_DST_PATH ]; then
	rm -f $START_SCRIPT_DST_PATH
fi

if [ -f $START_SCRIPT_DST_PATH_1 ]; then
	rm -f $START_SCRIPT_DST_PATH_1
fi
 
cp $START_SCRIPT_SRC_PATH $START_SCRIPT_DST_PATH
cp $START_SCRIPT_SRC_PATH $START_SCRIPT_DST_PATH_1

##################################################
# change core dump path
if [ ! -d /home/allcores ]; then
	mkdir /home/allcores
fi

echo "change core dump path"
sed -i "/kernel.core_pattern/d" /etc/sysctl.conf
echo "kernel.core_pattern=/home/allcores/core-%e-%p-%s-%t" >> /etc/sysctl.conf
sysctl -p

##################################################
# start program
echo "start program!"
$START_SCRIPT_DST_PATH



