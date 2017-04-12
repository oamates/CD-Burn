#ifndef __MKUDFFS_H__
#define __MKUDFFS_H__

#include "ecma_167.h"
#include "osta_udf.h"
#include "udf_endian.h"
#include "libudffs.h"
#include "udffscore.h"


#define CS0				0x00000001
#define UDF_ID_APPLICATION	"*Linux mkudffs"
#define MKUDFFS_VERSION		"1.0.0b2"

#define EMAIL_STRING		"<linux_udf@hpesjro.fc.hp.com>"

#define UDFFS_DATE			"2010/01/01"
#define UDFFS_VERSION		"1.0.0-cvs"

//π‚≈Ã¿‡–Õ
#define DEFAULT_HD	0
#define DEFAULT_DVD	0
#define DEFAULT_DVDRAM	0
#define DEFAULT_WORM	1
#define DEFAULT_MO		1
#define DEFAULT_CDRW	2
#define DEFAULT_CDR		3
#define DEFAULT_DVDRW	4

#ifndef NAME_MAX
#define NAME_MAX	255
#endif

void DVDUdf_UDF_Init_Disc(void *hMem, struct udf_disc *disc,uint64_t DisckBlocks, DISC_VOLID_T *pDiscVol);
void DVDUdf_Split_Space(void *hMem, struct udf_disc *disc);
int  DVDUdf_Setup_Fileset(void *hMem, struct udf_disc *disc, struct udf_extent *pspace, uint16_t DataMode);
int  DVDUdf_Setup_Root(void *hMem, struct udf_disc *disc, struct udf_extent *pspace, udfinfo_t *pUdfInfo);
void DVDUdf_Setup_VRS(void *hMem, struct udf_disc *disc);
void DVDUdf_Setup_Anchor(void *hMem, struct udf_disc *disc,int DataMode);
void DVDUdf_Setup_VDS(void *hMem, struct udf_disc *disc,uint16_t DataMode, udfinfo_t *pUdfInfo);
void DVDUdf_FreeUDF_Disc(void *hMem, struct udf_disc * disc);
int  DVDUdf_Insert_Desc(void *hMem, struct udf_desc *desc);

#endif//__MKUDFFS_H__
