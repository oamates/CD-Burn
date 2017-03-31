//cgccmd.h
#ifndef __CGCCMD_H__
#define __CGCCMD_H__

#define  MMCCMD_BLANK 					 		0xA1
#define  MMCCMD_CLOSE_TRACK_SESSION     		0x5B
#define  MMCCMD_ERASE 					 		0x2C
#define  MMCCMD_FORMAT_UNIT 					0x04
#define  MMCCMD_GET_CONFIGURATION 	     		0x46
#define  MMCCMD_GET_EVENT_STATUS_NOTIFICATION 	0x4A
#define  MMCCMD_GET_PERFORMANCE 		 		0xAC
#define  MMCCMD_INQUIRY 				 		0x12
#define  MMCCMD_LOAD_UNLOAD_MEDIUM 	 			0xA6
#define  MMCCMD_MECHANISM_STATUS 		 		0xBD
#define  MMCCMD_MODE_SELECT 			 		0x55
#define  MMCCMD_MODE_SENSE 			 			0x5A
#define  MMCCMD_PAUSE_RESUME 			 		0x4B
#define  MMCCMD_PLAY_AUDIO_10 					0x45
#define  MMCCMD_PLAY_AUDIO_12  					0xA5
#define  MMCCMD_PLAY_AUDIO_MSF  				0x47
#define  MMCCMD_PREVENT_ALLOW_MEDIUM_REMOVAL  	0x1E
#define  MMCCMD_READ_10 						0x28
#define  MMCCMD_READ_12  						0xA8
#define  MMCCMD_READ_BUFFER 					0x3C
#define  MMCCMD_READ_BUFFER_CAPACITY  			0x5C
#define  MMCCMD_READ_CAPACITY  					0x25
#define  MMCCMD_READ_CD  						0xBE
#define  MMCCMD_READ_CD_MSF  					0xB9
#define  MMCCMD_READ_DISC_INFORMATION  			0x51
#define  MMCCMD_READ_DVD_STRUCTURE  			0xAD
#define  MMCCMD_READ_DISC_STRUCTURE  			0xAD
#define  MMCCMD_READ_FORMAT_CAPACITIES  		0x23
#define  MMCCMD_READ_SUB_CHANNEL  				0x42
#define  MMCCMD_READ_TOC_PMA_ATIP  				0x43
#define  MMCCMD_READ_TRACK_INFORMATION  		0x52
#define  MMCCMD_REPAIR_TRACK  					0x58
#define  MMCCMD_REPORT_KEY  					0xA4
#define  MMCCMD_REQUEST_SENSE  					0x03
#define  MMCCMD_RESERVE_TRACK  					0x53
#define  MMCCMD_SCAN  							0xBA
#define  MMCCMD_SEEK_10  						0x2B
#define  MMCCMD_SEND_CUE_SHEET  				0x5D
#define  MMCCMD_SEND_DVD_STRUCTURE  			0xBF
#define  MMCCMD_SEND_KEY  						0xA3
#define  MMCCMD_SEND_OPC_INFORMATION  			0x54
#define  MMCCMD_SET_SPEED  						0xBB
#define  MMCCMD_SET_READ_AHEAD  				0xA7
#define  MMCCMD_SET_STREAMING  					0xB6
#define  MMCCMD_START_STOP_UNIT  				0x1B
#define  MMCCMD_STOP_PLAY_SCAN 					0x4E
#define  MMCCMD_SYNCHRONIZE_CACHE  				0x35
#define  MMCCMD_TEST_UNIT_READY  				0x00
#define  MMCCMD_VERIFY_10  						0x2F
#define  MMCCMD_WRITE_10  						0x2A
#define  MMCCMD_WRITE_12  						0xAA
#define  MMCCMD_WRITE_AND_VERIFY_10  			0x2E
#define  MMCCMD_WRITE_BUFFER  					0x3B

#define WAIT_PC			(5 * HZ)
#define WAIT_SYNC		(25 * HZ)
#define WAIT_BLANK		(60 * 60 * HZ)
#define WAIT_BLANK1		(7 * HZ)
#define WAIT_SPEED		(30 * HZ)
#define WAIT_BLANK2		(2000 * 60 * HZ)
#define WAIT_MAXTIME	0X7FFFFFF0

#endif//__CGCCMD_H__
