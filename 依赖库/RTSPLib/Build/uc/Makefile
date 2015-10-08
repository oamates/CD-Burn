#ChnsysRTSPLib Makefile
#

LIB_NAME = libChnsysRTSP.so
LIB_PATH = /rootfs/rtsp/

COMPILER_FLAGS	+= -D_LINUX_ 
COMPILER_FLAGS	+= -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D__USE_POSIX
COMPILER_FLAGS	+= -Wall -Wno-switch -Wpointer-arith -Wredundant-decls -pipe -ffast-math -fomit-frame-pointer

CCFLAGS += $(COMPILER_FLAGS)

CCFLAGS += -I.
CCFLAGS += -I..
CCFLAGS += -I../../Source/
CCFLAGS += -I../../Source/Protocol/
CCFLAGS += -I../../Source/Protocol/Base/
CCFLAGS += -I../../Source/Protocol/Packetizer/
CCFLAGS += -I../../Source/Protocol/Parser/
CCFLAGS += -I../../Source/Protocol/RTCP/
CCFLAGS += -I../../Source/Protocol/RTP/
CCFLAGS += -I../../Source/Protocol/RTSP/
CCFLAGS += -I../../Source/Protocol/SDP/
CCFLAGS += -I../../Depends/TaskModelLib/

C++FLAGS = $(CCFLAGS)

CPPFILES =  ../../Source/Protocol/Base/ZSessionStream.cpp\
			../../Source/Protocol/Base/ZSessionMessage.cpp\
			../../Source/Protocol/Base/ZSession.cpp\
			../../Source/Protocol/Base/ZResponseStream.cpp\
			../../Source/Protocol/Base/ZRequestStream.cpp\
			../../Source/Protocol/Base/ZProtocol.cpp\
			../../Source/Protocol/Base/ZHeaderParser.cpp\
			../../Source/Protocol/Base/ZAuthorization.cpp\
			../../Source/Protocol/Base/ZAuthenticator.cpp\
			../../Source/Protocol/Base/NalUtil.cpp\
			../../Source/Protocol/Packetizer/RTPPacketizer.cpp\
			../../Source/Protocol/Packetizer/Packetizer3984.cpp\
			../../Source/Protocol/Packetizer/Packetizer3640.cpp\
			../../Source/Protocol/Packetizer/Packetizer3016.cpp\
			../../Source/Protocol/Parser/RTPParser.cpp\
			../../Source/Protocol/Parser/Parser3984.cpp\
			../../Source/Protocol/Parser/Parser3640.cpp\
			../../Source/Protocol/Parser/Parser3016.cpp\
			../../Source/Protocol/Parser/AACUtil.cpp\
			../../Source/Protocol/RTCP/ZRTCPTask.cpp\
			../../Source/Protocol/RTCP/ZRTCPPacket.cpp\
			../../Source/Protocol/RTP/ZRTPStream.cpp\
			../../Source/Protocol/RTP/ZRTPSocketPool.cpp\
			../../Source/Protocol/RTP/ZRTPSession.cpp\
			../../Source/Protocol/RTP/ZRTPProtocol.cpp\
			../../Source/Protocol/RTP/ZRTPPacket.cpp\
			../../Source/Protocol/RTP/ZMultiAddrPool.cpp\
			../../Source/Protocol/RTP/MulticastPin.cpp\
			../../Source/Protocol/RTSP/ZRTSPSession.cpp\
			../../Source/Protocol/RTSP/ZRTSPResponse.cpp\
			../../Source/Protocol/RTSP/ZRTSPRequest.cpp\
			../../Source/Protocol/RTSP/ZRTSPProtocol.cpp\
			../../Source/Protocol/RTSP/ZRTSPMessage.cpp\
			../../Source/Protocol/RTSP/RTSPServerSource.cpp\
			../../Source/Protocol/RTSP/RTSPServerListenerManager.cpp\
			../../Source/Protocol/RTSP/RTSPServerListener.cpp\
			../../Source/Protocol/RTSP/RTSPClientSource.cpp\
			../../Source/Protocol/RTSP/RTSPClientListener.cpp\
			../../Source/Protocol/RTSP/RemoteRTSPClient.cpp\
			../../Source/Protocol/SDP/ZSessionDescription.cpp\
			../../Source/Protocol/SDP/SDPUtil.cpp\
			../../Source/ChnsysRTSP.cpp\
			
all: $(LIB_NAME)

$(LIB_NAME): $(CPPFILES:.cpp=.o)
	arm-hisiv100-linux-gcc -fpic -shared -o $(LIB_PATH)$(LIB_NAME) $(CPPFILES:.cpp=.o)

clean:
	rm -f $(LIB_PATH)$(LIB_NAME) $(CPPFILES:.cpp=.o)

.cpp.o:
	arm-hisiv100-linux-gcc -c -o $*.o $(C++FLAGS) $*.cpp
