
#ifndef __DEFAULTS_H__
#define __DEFAULTS_H__
#include "osta_udf.h"

typedef struct stSiconvTable siconvt;
struct stSiconvTable {
	char		*sic_name;				/* SICONV charset name	*/
	unsigned int	*sic_cs2uni;		/* Charset -> Unicode	*/
	unsigned char	**sic_uni2cs;		/* Unicode -> Charset	*/
	char *		sic_cd2uni;				/* iconv Charset -> Unicode */
	char *		sic_uni2cd;				/* iconv Unicode -> Charset */
	siconvt	*sic_alt;					/* alternate iconv tab	*/
	siconvt	*sic_next;					/* Next table		*/
};

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

class UDF_DefaultInfo
{
public:
	UDF_DefaultInfo();
	UDF_DefaultInfo(PrimaryVolDesc* pPVD,
		LogicalVolDesc* pLVD, VolDescPtr* pVDP,
		ImpUseVolDescImpUse* pIUVDIU, ImpUseVolDesc* pIUVD,
		PartitionDesc* pPD, UnallocSpaceDesc* pUSD,
		TerminatingDesc* pTD, LogicalVolIntegrityDesc* pLVID,
		LogicalVolIntegrityDescImpUse* pLVIDI, SparingTable* pST,
		SparablePartitionMap* pSPM, VirtualAllocationTable15* pVAT15,
		VirtualAllocationTable20* pVAT20, VirtualPartitionMap* pVPM,
		FileSetDesc* pFSD, FileEntry* pFE, ExtendedFileEntry* pEFE);
	~UDF_DefaultInfo(){};
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

extern UDF_DefaultInfo UDF_Defaults;

#endif//__DEFAULTS_H__
