#!/bin/sh

cd /home
export LD_LIBRARY_PATH=/root/Communicate_Schedule_exe/lib_rtsp:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/root/Communicate_Schedule_exe/lib_freetype:$LD_LIBRARY_PATH
./TestRTSPServer &


