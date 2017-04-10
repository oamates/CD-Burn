
#ifndef __DEFAULTS_H__
#define __DEFAULTS_H__
#include "osta_udf.h"

typedef struct siconv_table	siconvt_t;
struct siconv_table {
	char		*sic_name;		/* SICONV charset name	*/
	unsigned int	*sic_cs2uni;		/* Charset -> Unicode	*/
	unsigned char	**sic_uni2cs;		/* Unicode -> Charset	*/
	char *		sic_cd2uni;		/* iconv Charset -> Unicode */
	char *		sic_uni2cd;		/* iconv Unicode -> Charset */
	siconvt_t	*sic_alt;		/* alternate iconv tab	*/
	siconvt_t	*sic_next;		/* Next table		*/
};

#if 0
typedef struct{
	struct primaryVolDesc      			 *default_pvd;
	struct logicalVolDesc      			 *default_lvd;
	struct volDescPtr          			 *default_vdp;
	struct impUseVolDescImpUse 			 *default_iuvdiu;
	struct impUseVolDesc       			 *default_iuvd;
	struct partitionDesc       			 *default_pd;
	struct unallocSpaceDesc    			 *default_usd;
	struct terminatingDesc            	 *default_td;
	struct logicalVolIntegrityDesc       *default_lvid;
	struct logicalVolIntegrityDescImpUse *default_lvidiu;
	struct sparingTable 			  	 *default_stable;
	struct sparablePartitionMap 		 *default_sparmap;
	struct virtualAllocationTable15 	 *default_vat15;
	struct virtualAllocationTable20 	 *default_vat20;
	struct virtualPartitionMap 			 *default_virtmap;
	struct fileSetDesc 					 *default_fsd;
	struct fileEntry 					 *default_fe;
	struct extendedFileEntry 			 *default_efe;
}LVUDF_DEFAULTSINFO_T;
#endif

class PrimaryVolDesc
{
public:
	PrimaryVolDesc();
	~PrimaryVolDesc();
public:
	struct primaryVolDesc default_pvd;
};

class LogicalVolDesc;
class VolDescPtr;
class ImpUseVolDescImpUse;
class ImpUseVolDesc;
class PartitionDesc;
class UnallocSpaceDesc;
class TerminatingDesc;
class LogicalVolIntegrityDesc;

class LogicalVolIntegrityDescImpUse
{
public:
	LogicalVolIntegrityDescImpUse();
	~LogicalVolIntegrityDescImpUse(){};
public:
	struct logicalVolIntegrityDescImpUse default_lvidiu;
};

class SparingTable;
class SparablePartitionMap;
class VirtualAllocationTable15;
class VirtualAllocationTable20;
class VirtualPartitionMap;
class FileSetDesc;

class FileEntry
{
public:
	FileEntry();
	~FileEntry(){};
public:
	fileEntry default_fe;
};

//class FileEntry;
class ExtendedFileEntry;

/*typedef struct{
	PrimaryVolDesc      			 *default_pvd;
	LogicalVolDesc      			 *default_lvd;
	VolDescPtr          			 *default_vdp;
	ImpUseVolDescImpUse 			 *default_iuvdiu;
	ImpUseVolDesc       			 *default_iuvd;
	PartitionDesc       			 *default_pd;
	UnallocSpaceDesc    			 *default_usd;
	TerminatingDesc            	 *default_td;
	LogicalVolIntegrityDesc       *default_lvid;
	LogicalVolIntegrityDescImpUse *default_lvidiu;
	SparingTable 			  	 *default_stable;
	SparablePartitionMap 		 *default_sparmap;
	VirtualAllocationTable15 	 *default_vat15;
	VirtualAllocationTable20 	 *default_vat20;
	VirtualPartitionMap 			 *default_virtmap;
	FileSetDesc 					 *default_fsd;
	FileEntry 					 *default_fe;
	ExtendedFileEntry 			 *default_efe;
}LVUDF_DEFAULTSINFO_T;*/

class LVUDF_DEFAULTSINFO
{
public:
	LVUDF_DEFAULTSINFO();
	LVUDF_DEFAULTSINFO(PrimaryVolDesc* pPVD,
		LogicalVolDesc* pLVD, VolDescPtr* pVDP,
		ImpUseVolDescImpUse* pIUVDIU, ImpUseVolDesc* pIUVD,
		PartitionDesc* pPD, UnallocSpaceDesc* pUSD,
		TerminatingDesc* pTD, LogicalVolIntegrityDesc* pLVID,
		LogicalVolIntegrityDescImpUse* pLVIDI, SparingTable* pST,
		SparablePartitionMap* pSPM, VirtualAllocationTable15* pVAT15,
		VirtualAllocationTable20* pVAT20, VirtualPartitionMap* pVPM,
		FileSetDesc* pFSD, FileEntry* pFE, ExtendedFileEntry* pEFE);
	~LVUDF_DEFAULTSINFO(){};
public:
	PrimaryVolDesc      			 *default_pvd;
	LogicalVolDesc      			 *default_lvd;
	VolDescPtr          			 *default_vdp;
	ImpUseVolDescImpUse 			 *default_iuvdiu;
	ImpUseVolDesc       			 *default_iuvd;
	PartitionDesc       			 *default_pd;
	UnallocSpaceDesc    			 *default_usd;
	TerminatingDesc            	 *default_td;
	LogicalVolIntegrityDesc       *default_lvid;
	LogicalVolIntegrityDescImpUse *default_lvidiu;
	SparingTable 			  	 *default_stable;
	SparablePartitionMap 		 *default_sparmap;
	VirtualAllocationTable15 	 *default_vat15;
	VirtualAllocationTable20 	 *default_vat20;
	VirtualPartitionMap 			 *default_virtmap;
	FileSetDesc 					 *default_fsd;
	FileEntry 					 *default_fe;
	ExtendedFileEntry 			 *default_efe;
};

extern LVUDF_DEFAULTSINFO LvUDF_Defaults;

#endif//__DEFAULTS_H__
