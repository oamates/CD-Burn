//debug.h
#ifndef __DEBUG_H__
#define __DEBUG_H__

#if defined(DEBUG) || defined(_DEBUG)

	#if defined(WIN32)
		#define DP(X) printf X
		#define DPERROR(X) printf("error(FILE:%s LINE:%d) ",__FILE__, __LINE__); printf X
		
	#elif defined(LINUX)
		#define DP(X) printf X
		#define DPERROR(X) printf("error(FILE:%s LINE:%d) ",__FILE__, __LINE__); printf X
	#elif defined(SPI_TARGET_DEVICE)
		#define DP(X) spi_printf X
		#define DPERROR(X) spi_printf("error(FILE:%s LINE:%d) ",__FILE__, __LINE__); spi_printf X
	#else
		#error "asdfasdfasd"
	#endif//
#else
	#define DP(X)
	#define DPERROR(X)
	#define DFP(X)
	#define DNP(X)
	#define DFPERR(X)
	#define DNPERR(X)
#endif //DEBUG

#endif//__DEBUG_H__
