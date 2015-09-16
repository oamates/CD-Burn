#ifndef __BURN_ERR_H__
#define __BURN_ERR_H__

//DVD_SDK返回错误码
#define ERROR_DVD_OK				0				//成功
#define ERROR_DVD_NODEV				_ERRC(300)		// 设备不存在
#define ERROR_DVD_ERRDEVNO			_ERRC(301)		// 错误的设备号
#define ERROR_DVD_BUSY				_ERRC(302)		// 光驱正忙
#define ERROR_DVD_OPTERFAILED		_ERRC(303)		// 光驱操作失败
#define ERROR_DVD_NODISC			_ERRC(304)		// 光驱无CD
#define ERROR_DVD_UNKNOWNDISCTYPE	_ERRC(305)		// 未知的CD类型(不支持该光盘类型)
#define ERROR_DVD_UNBLANKDISC		_ERRC(306)		// 不是空白盘
#define ERROR_DVD_RESERVETRACKERR	_ERRC(307)		// 保留轨道错误
#define ERROR_DVD_CDNOFILES			_ERRC(308)		// 没有找到文件
#define ERROR_DVD_FORMATFAILED		_ERRC(309)		// 格式化光盘失败
#define ERROR_DVD_OPENFILEERROR		_ERRC(310)		// 打开文件失败
#define ERROR_DVD_WRITEERROR		_ERRC(311)		// 写盘发生错误
#define ERROR_DVD_DISCNOFREESIZE	_ERRC(312)		// 无可用空间
#define ERROR_DVD_LOADDISCFAILED	_ERRC(313)		// 加载失败
#define ERROR_DVD_NAMEEMPTY			_ERRC(314)		// 名称为空
#define ERROR_DVD_NAMEEXIST			_ERRC(315)		// 名称已经存在
#define ERROR_DVD_DISCDIFFTYPE		_ERRC(316)		// 光盘类型不同
#define ERROR_DVD_CANTRESUMEBLANK	_ERRC(317)		// 光盘为空盘
#define ERROR_DVD_CANTRESUMEDISC    _ERRC(318)		// 不能恢复的光盘
#define ERROR_DVD_CANTCOPYDISC      _ERRC(319)		// 不能光盘容量不同不能复制
#define ERROR_DVD_SRCBLANKDISC		_ERRC(320)		// 源盘为空盘

#endif
