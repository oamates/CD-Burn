//drvcomm.h
#ifndef __DRVCOMM_H__
#define __DRVCOMM_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <asm/param.h>
#include <linux/cdrom.h>
#include <pthread.h>
#include "cgccmd.h"
#include "bswap.h"

#ifndef BOOL
	#define BOOL	int
#endif//BOOL

#ifndef TRUE
	#define TRUE	1
#endif//TRUE

#ifndef FALSE
	#define FALSE	0
#endif//FALSE

#define CDROM_BLOCK			2048  // 一个DVD扇区2048个字节
#define BLOCK_OFFSETBITS	11	  // << 11 = * 2048
#define PACKET_BLOCK_16		16    // UDF要求连续16个扇区为一个包块字节
#define PACKET_BLOCK_32		32    // UDF要求连续32个扇区为一个包块字节
#define PACKET_BLOCK_64		64    // UDF要求连续64个扇区为一个包块字节

#define PACKET16_SIZE		(CDROM_BLOCK * PACKET_BLOCK_16)  //默认包大小为16扇区 * 2048
#define PACKET32_SIZE		(CDROM_BLOCK * PACKET_BLOCK_32)  //默认包大小为32扇区 * 2048


#define BIGFILERECORD   1073739776//0X3FFFF800          //大文件时需要分块记录数据(>1g)
#define BIGFILENUM      524287              //占用sec数量
#define FILE1G          0X40000000          //大于1g

#define CD_SPEED_X1     176       //CD一倍速
#define DVD_SPEED_X1    1385      //DVD一倍速
#define BD_SPEED_X1     4495      //BD一倍速

#ifndef NAME_MAX
#define NAME_MAX	255
#endif

#if defined(__CUP_BIG_ENDIAN)
#error "error for dinfine __CUP_BIG_ENDIAN"
#endif//__CUP_BIG_ENDIAN

//枚举光盘状态
// disc_info_t.status
enum{
	DISCSTE_BLANK 		= 0,	// 空盘
	DISCSTE_INCOMPLETE  = 1,	// 不完整光盘，需要恢复
	DISCSTE_COMPLETE 	= 2,	// 完整光盘
	DISCSTE_OTHER 		= 3,	// 其他
};

//光盘基本类型
enum 
{
	B_DVD_DISC  = 0 ,
	DVD_DL_DISC = 1 ,   
	DVD_DISC    = 2 ,
	CD_DISC     = 3
};


#define 	PROFILE_HD_DVD_RAM      0x52
#define 	PROFILE_HD_DVD_R        0x51
#define 	PROFILE_HD_DVD_ROM      0x50
#define 	PROFILE_BD_RE           0x43
#define 	PROFILE_BD_R2           0x42
#define 	PROFILE_BD_R1           0x41
#define 	PROFILE_BD_ROM          0x40
#define 	PROFILE_DVD_R_P_D       0x2b
#define 	PROFILE_DVD_RW_P_D      0x2a
#define 	PROFILE_DVD_R_P         0x1b
#define 	PROFILE_DVD_RW_P        0x1a
#define 	PROFILE_DVD_D           0x18
#define 	PROFILE_DVD_R_D         0x16
#define 	PROFILE_DVD_R_D1        0x15
#define 	PROFILE_DVD_RW2         0x14
#define 	PROFILE_DVD_RW1         0x13
#define 	PROFILE_DVD_RAM         0x12
#define 	PROFILE_DVD_R           0x11
#define 	PROFILE_DVD_ROM         0x10
#define 	PROFILE_CD_RW           0x0a
#define 	PROFILE_CD_R            0x09
#define 	PROFILE_CD_ROM          0x08
#define 	PROFILE_REMOVABLE       0x02
#define 	PROFILE_NO_CURRENT      0x00


typedef struct{
	uint16_t info_length;
	uint8_t track_number_l;
	uint8_t session_number_l;
	uint8_t reserved1;
#if defined(__CUP_BIG_ENDIAN)
	uint8_t reserved2	: 2;
	uint8_t damage		: 1;
	uint8_t copy		: 1;
	uint8_t track_mode	: 4;
	uint8_t rt			: 1;
	uint8_t blank		: 1;
	uint8_t packet		: 1;
	uint8_t fp			: 1;
	uint8_t data_mode	: 4;
	uint8_t reserved3	: 6;
	uint8_t lra_v		: 1;
	uint8_t nwa_v		: 1;
#else
	uint8_t track_mode	: 4;
	uint8_t copy		: 1;
	uint8_t damage		: 1;
	uint8_t reserved2	: 2;
	uint8_t data_mode	: 4;
	uint8_t fp			: 1;
	uint8_t packet		: 1;
	uint8_t blank		: 1;
	uint8_t rt			: 1;
	uint8_t nwa_v		: 1;
	uint8_t lra_v		: 1;
	uint8_t reserved3	: 6;
#endif//__CUP_BIG_ENDIAN
	uint32_t track_start;
	uint32_t next_writable;
	uint32_t free_blocks;
	uint32_t packet_size;
	uint32_t track_size;
	uint32_t last_recorded;
	uint8_t track_number_m;
	uint8_t session_number_m;
	uint8_t reserved4;
	uint8_t reserved5;
}track_info_t;

typedef struct{
	uint16_t length;
#if defined(__CUP_BIG_ENDIAN)
	uint8_t reserved1	: 3;
	uint8_t erasable	: 1;
	uint8_t border		: 2;
	uint8_t status		: 2;
#else
	uint8_t status		: 2;
	uint8_t border		: 2;
	uint8_t erasable	: 1;
	uint8_t reserved1	: 3;
#endif//__CUP_BIG_ENDIAN

	uint8_t n_first_track;
	uint8_t n_sessions_l;
	uint8_t first_track_l;
	uint8_t last_track_l;
	
#if defined(__CUP_BIG_ENDIAN)
	uint8_t did_v		: 1;
	uint8_t dbc_v		: 1;
	uint8_t uru			: 1;
	uint8_t reserved2	: 5;
#else
	uint8_t reserved2	: 5;
	uint8_t uru			: 1;
	uint8_t dbc_v		: 1;
	uint8_t did_v		: 1;
#endif//__CUP_BIG_ENDIAN

	uint8_t disc_type;
	uint8_t n_sessions_m;
	uint8_t first_track_m;
	uint8_t last_track_m;
	uint32_t disc_id;
	uint8_t lead_in_r;
	uint8_t lead_in_m;
	uint8_t lead_in_s;
	uint8_t lead_in_f;
	uint8_t lead_out_r;
	uint8_t lead_out_m;
	uint8_t lead_out_s;
	uint8_t lead_out_f;
	uint8_t disc_bar_code[8];
	uint8_t reserved3;
	uint8_t opc_entries;
}disc_info_t;

typedef struct{
	uint8_t   write_type;		/* Session write type (PACKET/TAO...)*/
	uint8_t	  ls_v;				/* link_size valid */
	uint8_t	  border;			/* or session */
	uint8_t	  fpacket;			/* fixed, variable */
	uint8_t	  track_mode;		/* control nibbler, sub q */
	uint8_t	  data_block;		/* write type */
	uint8_t	  link_size;		/* links loss size */
	uint8_t	  session_format;	/* session closure settings */
	uint8_t   multi_session;	
	uint32_t  packet_size;		/* fixed packet size */
	uint32_t  MaxReadSpeed;
	uint32_t  MaxWriteSpeed;
	uint32_t  WriteMode;
	uint8_t   subhdr2;
}write_params_t;

typedef struct{
	uint8_t res0;                   /* Reserved                          */
#if defined(__CUP_BIG_ENDIAN)
	uint8_t res_1_27        : 6;    /* Reserved                          */
	uint8_t rot_ctl_sel     : 2;    /* Rotational control selected       */
#else
	uint8_t rot_ctl_sel     : 2;    /* Rotational control selected       */
	uint8_t res_1_27        : 6;    /* Reserved                          */
#endif//__CUP_BIG_ENDIAN
	uint8_t wr_speed_supp[2];       /* Supported write speed             */
}cd_wr_speed_performance;

#if defined(__CUP_BIG_ENDIAN)
struct mm_cap_page_2A {
	uint8_t PS               : 1;
	uint8_t res_1            : 1;
	uint8_t page_code        : 6;
	uint8_t page_len;             /* 0x14 = 20 Bytes (MMC) */
	/* 0x18 = 24 Bytes (MMC-2) */
	/* 0x1C >= 28 Bytes (MMC-3) */
	uint8_t res_2_67         : 2; /* Reserved        */
	uint8_t dvd_ram_read     : 1; /* Reads DVD-RAM media       */
	uint8_t dvd_r_read       : 1; /* Reads DVD-R media       */
	uint8_t dvd_rom_read     : 1; /* Reads DVD ROM media       */
	uint8_t method2          : 1; /* Reads fixed packet method2 media  */
	uint8_t cd_rw_read       : 1; /* Reads CD-RW media       */
	uint8_t cd_r_read        : 1; /* Reads CD-R  media       */
	uint8_t res_3_67         : 2; /* Reserved        */
    uint8_t dvd_ram_write    : 1; /* Supports writing DVD-RAM media    */
    uint8_t dvd_r_write      : 1; /* Supports writing DVD-R media      */
    uint8_t res_3_3          : 1; /* Reserved        */
    uint8_t test_write       : 1; /* Supports emulation write      */
    uint8_t cd_rw_write      : 1; /* Supports writing CD-RW media      */
    uint8_t cd_r_write       : 1; /* Supports writing CD-R  media      */
    uint8_t BUF              : 1; /* Supports Buffer under. free rec.  */
    uint8_t multi_session    : 1; /* Reads multi-session media      */
    uint8_t mode_2_form_2    : 1; /* Reads Mode-2 form 2 media      */
    uint8_t mode_2_form_1    : 1; /* Reads Mode-2 form 1 media (XA)    */
    uint8_t digital_port_1   : 1; /* Supports digital output on port 1 */
    uint8_t digital_port_2   : 1; /* Supports digital output on port 2 */
	uint8_t composite        : 1; /* Deliveres composite A/V stream    */
	uint8_t audio_play       : 1; /* Supports Audio play operation     */
	uint8_t read_bar_code    : 1; /* Supports reading bar codes      */
	uint8_t UPC              : 1; /* Reads media catalog number (UPC)  */
	uint8_t ISRC             : 1; /* Reads ISRC information      */
	uint8_t c2_pointers      : 1; /* Supports C2 error pointers      */
	uint8_t rw_deint_corr    : 1; /* Reads de-interleved R-W sub chan  */
	uint8_t rw_supported     : 1; /* Reads R-W sub channel information */
	uint8_t cd_da_accurate   : 1; /* READ CD data stream is accurate   */
	uint8_t cd_da_supported  : 1; /* Reads audio data with READ CD cmd */
	uint8_t loading_type     : 3; /* Loading mechanism type      */
	uint8_t res_6_4          : 1; /* Reserved        */
	uint8_t eject            : 1; /* Ejects disc/cartr with STOP LoEj  */
	uint8_t prevent_jumper   : 1; /* State of prev/allow jumper 0=pres */
	uint8_t lock_state       : 1; /* Lock state 0=unlocked 1=locked    */
	uint8_t lock             : 1; /* PREVENT/ALLOW may lock media      */
	uint8_t res_7            : 2; /* Reserved        */
	uint8_t rw_in_lead_in    : 1; /* Reads raw R-W subcode from lead in */
	uint8_t side_change      : 1; /* Side change capable       */
	uint8_t sw_slot_sel      : 1; /* Load empty slot in changer      */
	uint8_t disk_present_rep : 1; /* Changer supports disk present rep */
	uint8_t sep_chan_mute    : 1; /* Mute controls each channel separat*/
	uint8_t sep_chan_vol     : 1; /* Vol controls each channel separat */
	uint8_t max_read_speed[2];    /* Max. read speed in KB/s      */
	/* obsolete in MMC-4 */
	uint8_t num_vol_levels[2];    /* # of supported volume levels      */
	uint8_t buffer_size[2];       /* Buffer size for the data in KB    */
	uint8_t cur_read_speed[2];    /* Current read speed in KB/s      */
	/* obsolete in MMC-4 */
	uint8_t res_16;               /* Reserved        */
	uint8_t res_17           : 2; /* Reserved        */
	uint8_t length           : 2; /* 0=32BCKs 1=16BCKs 2=24BCKs 3=24I2c*/
	uint8_t LSBF             : 1; /* Set: LSB first Clear: MSB first   */
	uint8_t RCK              : 1; /* Set: HIGH high LRCK=left channel  */
	uint8_t BCK              : 1; /* Data valid on falling edge of BCK */
	uint8_t res_17_0         : 1; /* Reserved        */
	uint8_t max_write_speed[2];   /* Max. write speed supported in KB/s*/
	/* obsolete in MMC-4 */
	uint8_t cur_write_speed[2];   /* Current write speed in KB/s      */
	/* obsolete in MMC-4 */

	/* Byte 22 ... Only in MMC-2      */
	uint8_t copy_man_rev[2];      /* Copy management revision supported*/
	uint8_t res_24;               /* Reserved        */
	uint8_t res_25;               /* Reserved        */

	/* Byte 26 ... Only in MMC-3      */
	uint8_t res_26;               /* Reserved        */
	uint8_t res_27_27        : 6; /* Reserved        */
	uint8_t rot_ctl_sel      : 2; /* Rotational control selected      */
	uint8_t v3_cur_write_speed[2]; /* Current write speed in KB/s      */
	uint8_t num_wr_speed_des[2];  /* # of wr speed perf descr. tables  */
	cd_wr_speed_performance 
		    wr_speed_des[1];             /* wr speed performance descriptor   */
	/* Actually more (num_wr_speed_des)  */
};
#else
struct mm_cap_page_2A {
	uint8_t page_code        : 6;
	uint8_t res_1            : 1;
	uint8_t PS               : 1;
	uint8_t page_len;             /* 0x14 = 20 Bytes (MMC) */
	/* 0x18 = 24 Bytes (MMC-2) */
	/* 0x1C >= 28 Bytes (MMC-3) */
	uint8_t cd_r_read        : 1; /* Reads CD-R  media       */
	uint8_t cd_rw_read       : 1; /* Reads CD-RW media       */
	uint8_t method2          : 1; /* Reads fixed packet method2 media  */
	uint8_t dvd_rom_read     : 1; /* Reads DVD ROM media       */
	uint8_t dvd_r_read       : 1; /* Reads DVD-R media       */
	uint8_t dvd_ram_read     : 1; /* Reads DVD-RAM media       */
	uint8_t res_2_67         : 2; /* Reserved        */
	uint8_t cd_r_write       : 1; /* Supports writing CD-R  media      */
	uint8_t cd_rw_write      : 1; /* Supports writing CD-RW media      */
	uint8_t test_write       : 1; /* Supports emulation write      */
	uint8_t res_3_3          : 1; /* Reserved        */
	uint8_t dvd_r_write      : 1; /* Supports writing DVD-R media      */
	uint8_t dvd_ram_write    : 1; /* Supports writing DVD-RAM media    */
	uint8_t res_3_67         : 2; /* Reserved        */
	uint8_t audio_play       : 1; /* Supports Audio play operation     */
	uint8_t composite        : 1; /* Deliveres composite A/V stream    */
	uint8_t digital_port_2   : 1; /* Supports digital output on port 2 */
	uint8_t digital_port_1   : 1; /* Supports digital output on port 1 */
	uint8_t mode_2_form_1    : 1; /* Reads Mode-2 form 1 media (XA)    */
	uint8_t mode_2_form_2    : 1; /* Reads Mode-2 form 2 media      */
	uint8_t multi_session    : 1; /* Reads multi-session media      */
	uint8_t BUF              : 1; /* Supports Buffer under. free rec.  */
	uint8_t cd_da_supported  : 1; /* Reads audio data with READ CD cmd */
	uint8_t cd_da_accurate   : 1; /* READ CD data stream is accurate   */
	uint8_t rw_supported     : 1; /* Reads R-W sub channel information */
	uint8_t rw_deint_corr    : 1; /* Reads de-interleved R-W sub chan  */
	uint8_t c2_pointers      : 1; /* Supports C2 error pointers      */
	uint8_t ISRC             : 1; /* Reads ISRC information      */
	uint8_t UPC              : 1; /* Reads media catalog number (UPC)  */
	uint8_t read_bar_code    : 1; /* Supports reading bar codes      */
	uint8_t lock             : 1; /* PREVENT/ALLOW may lock media      */
	uint8_t lock_state       : 1; /* Lock state 0=unlocked 1=locked    */
	uint8_t prevent_jumper   : 1; /* State of prev/allow jumper 0=pres */
	uint8_t eject            : 1; /* Ejects disc/cartr with STOP LoEj  */
	uint8_t res_6_4          : 1; /* Reserved        */
	uint8_t loading_type     : 3; /* Loading mechanism type      */
	uint8_t sep_chan_vol     : 1; /* Vol controls each channel separat */
	uint8_t sep_chan_mute    : 1; /* Mute controls each channel separat*/
	uint8_t disk_present_rep : 1; /* Changer supports disk present rep */
	uint8_t sw_slot_sel      : 1; /* Load empty slot in changer      */
	uint8_t side_change      : 1; /* Side change capable       */
	uint8_t rw_in_lead_in    : 1; /* Reads raw R-W subcode from lead in */
	uint8_t res_7            : 2; /* Reserved        */
	uint8_t max_read_speed[2];    /* Max. read speed in KB/s      */
	/* obsolete in MMC-4 */
	uint8_t num_vol_levels[2];    /* # of supported volume levels      */
	uint8_t buffer_size[2];       /* Buffer size for the data in KB    */
	uint8_t cur_read_speed[2];    /* Current read speed in KB/s      */
	/* obsolete in MMC-4 */
	uint8_t res_16;               /* Reserved        */
	uint8_t res_17_0         : 1; /* Reserved        */
	uint8_t BCK              : 1; /* Data valid on falling edge of BCK */
	uint8_t RCK              : 1; /* Set: HIGH high LRCK=left channel  */
	uint8_t LSBF             : 1; /* Set: LSB first Clear: MSB first   */
	uint8_t length           : 2; /* 0=32BCKs 1=16BCKs 2=24BCKs 3=24I2c*/
	uint8_t res_17           : 2; /* Reserved        */
	uint8_t max_write_speed[2];   /* Max. write speed supported in KB/s*/
	/* obsolete in MMC-4 */
	uint8_t cur_write_speed[2];   /* Current write speed in KB/s      */
	/* obsolete in MMC-4 */

	/* Byte 22 ... Only in MMC-2      */
	uint8_t copy_man_rev[2];      /* Copy management revision supported*/
	uint8_t res_24;               /* Reserved        */
	uint8_t res_25;               /* Reserved        */

	/* Byte 26 ... Only in MMC-3      */
	uint8_t res_26;               /* Reserved        */
	uint8_t rot_ctl_sel      : 2; /* Rotational control selected      */
	uint8_t res_27_27        : 6; /* Reserved        */
	uint8_t v3_cur_write_speed[2]; /* Current write speed in KB/s      */
	uint8_t num_wr_speed_des[2];   /* # of wr speed perf descr. tables  */
	cd_wr_speed_performance
            wr_speed_des[1];             /* wr speed performance descriptor   */
	/* Actually more (num_wr_speed_des)  */
};
#endif//__CUP_BIG_ENDIAN

typedef struct{
	uint8_t      bclosed;    	// 轨道关闭标志
	uint8_t      trackmode;  	// 轨道模式
	uint8_t      datamode;   	// 数据模式
	uint8_t      trackid;    	// 轨道编号
	uint8_t      sessionid;   	// session编号
	uint32_t     writestart; 	// 起始位置
	uint32_t     writenext;  	// 下一个可写位置
	uint32_t     freeblocks; 	// 剩余扇区
	uint32_t     writedsize;    // 已经使用了多少容量
	uint32_t     tracksize;     // 轨道容量
	
	uint8_t		*pbuffer;		// 临时Buffer，用于轨道写入临时缓存(根据不同光驱有所不同: 32K, 64K)
	int			 buffsize;		// Buffer已有大小
	uint32_t     filldata;      //在刻录数据前做的填充(填充0数据)
}CDR_TRACK_T;

struct CDR_CMD_T{
	BOOL (*cdr_lockdoor)(int fd);		// 锁定托盘
	BOOL (*cdr_unlockdoor)(int fd);		// 解锁托盘
	BOOL (*cdr_opentray)(int fd);		// 打开托盘
	BOOL (*cdr_closetray)(int fd);		// 关闭托盘
	BOOL (*cdr_havedisc)(int fd);		// 判断是否有光盘
	int	 (*cdr_formatdisc)(int fd, int ReserveBlocks);									// 格式化光盘
	int  (*cdr_buffcap)(int fd, uint32_t *buffsize, uint32_t *bufffree);				// 获得光驱Buffer能力
	int  (*cdr_discbasictype)(int fd, int *type);										// 获得光盘基本类型
	int  (*cdr_discexacttype)(int fd, int *type);										// 获得光盘精确类型
	int  (*cdr_getdiscinfo)(int fd, int *TrickCount, int *SessionCount, uint64_t *Capicity,	int *DiskStatus);
	int  (*cdr_devcapablity)(int fd, int *capablity);									// 获得光驱能力
	int  (*cdr_loadmedia)(int fd);														// 加载媒体
	int  (*cdr_readtrack)(int fd, int start, uint8_t *pbuffer, int size);				// 读轨道数据
	int  (*cdr_writetrack)(int fd, CDR_TRACK_T *ptrack, uint8_t *pbuffer, int size);	// 写轨道数据, size可以为任意大小
	int  (*cdr_flushtrack)(int fd, CDR_TRACK_T *ptrack);								// 刷新全部数据到轨道
	int  (*cdr_pausewrite)(int fd);														// 暂停数据写入
	int  (*cdr_resumewrite)(int fd);													// 恢复数据写入
	int  (*cdr_gettrackinfo)(int fd, int trackid, CDR_TRACK_T *ptrack);					// 获得轨道信息
	int  (*cdr_closetrack)(int fd, CDR_TRACK_T *ptrack);								// 关闭轨道
	int  (*cdr_closesession)(int fd, CDR_TRACK_T *ptrack);								// 关闭session
	int  (*cdr_dpdevprofile)(int fd);													// 打印光驱profile
	int  (*cdr_getdevinfo)(int fd, DVD_DEV_INFO_T * pDevInfo);	                    // 获取光驱厂商
	int  (*cdr_getmaxspeed)(int fd,int *MaxReadSpeed,int *MaxWriteSpeed);               // 获取光盘最大速度 
	int  (*cdr_setspeed)(int fd,int ReadSpeed,int WriteSpeed,int DiscType);             // 设定最大速度 
	int  (*cdr_getdiscusedsize)(int fd,int * usedsize);                                 // 获取光盘使用容量
	int  (*cdr_getdoorstate)(int fd);                                                   //获取仓门状态
};

class CCDRCmd
{
public:
	CCDRCmd();
	~CCDRCmd();
public:
	BOOL (*CDR_LockDoor)(int fd);		// 锁定托盘
	BOOL (*CDR_UnlockDoor)(int fd);		// 解锁托盘
	BOOL (*CDR_OpenTray)(int fd);		// 打开托盘
	BOOL (*CDR_CloseTray)(int fd);		// 关闭托盘
	BOOL (*CDR_HaveDisc)(int fd);		// 判断是否有光盘
	int (*CDR_FormatDisc)(int fd, int ReserveBlocks);									// 格式化光盘
	int (*CDR_Buffcap)(int fd, uint32_t *buffsize, uint32_t *bufffree);				// 获得光驱Buffer能力
	int (*CDR_DiscBasicType)(int fd, int *type);										// 获得光盘基本类型
	int (*CDR_DiscExactType)(int fd, int *type);										// 获得光盘精确类型
	int (*CDR_GetDiscInfo)(int fd, int *TrickCount, int *SessionCount, uint64_t *Capicity, int *DiskStatus);
	int (*CDR_DevcapAblity)(int fd, int *capablity);									// 获得光驱能力
	int (*CDR_LoadMedia)(int fd);														// 加载媒体
	int (*CDR_ReadTrack)(int fd, int start, uint8_t *pbuffer, int size);				// 读轨道数据
	int (*CDR_WriteTrack)(int fd, CDR_TRACK_T *ptrack, uint8_t *pbuffer, int size);	// 写轨道数据, size可以为任意大小
	int (*CDR_FlushTrack)(int fd, CDR_TRACK_T *ptrack);								// 刷新全部数据到轨道
	int (*CDR_PauseWrite)(int fd);														// 暂停数据写入
	int (*CDR_ResumeWrite)(int fd);													// 恢复数据写入
	int (*CDR_GetTrackinfo)(int fd, int trackid, CDR_TRACK_T *ptrack);					// 获得轨道信息
	int (*CDR_CloseTrack)(int fd, CDR_TRACK_T *ptrack);								// 关闭轨道
	int (*CDR_CloseSession)(int fd, CDR_TRACK_T *ptrack);								// 关闭session
	int (*CDR_DpdevProfile)(int fd);													// 打印光驱profile
	int (*CDR_GetdevInfo)(int fd, DVD_DEV_INFO_T * pDevInfo);	                    // 获取光驱厂商
	int (*CDR_GetMaxSpeed)(int fd, int *MaxReadSpeed, int *MaxWriteSpeed);               // 获取光盘最大速度 
	int (*CDR_SetSpeed)(int fd, int ReadSpeed, int WriteSpeed, int DiscType);             // 设定最大速度 
	int (*CDR_GetDiscUsedSize)(int fd, int * usedsize);                                 // 获取光盘使用容量
	int (*CDR_GetDoorState)(int fd);                                                   //获取仓门状态
};

#endif//__DRVCOMM_H__
