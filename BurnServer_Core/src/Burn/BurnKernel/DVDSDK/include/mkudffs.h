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

void LvDVDUdf_udf_init_disc(void *hMem, struct udf_disc *disc,uint64_t DisckBlocks, DISC_VOLID_T *pDiscVol);
void LvDVDUdf_split_space(void *hMem, struct udf_disc *disc);
int  LvDVDUdf_setup_fileset(void *hMem, struct udf_disc *disc, struct udf_extent *pspace, uint16_t DataMode);
int  LvDVDUdf_setup_root(void *hMem, struct udf_disc *disc, struct udf_extent *pspace, udfinfo_t *pUdfInfo);
void LvDVDUdf_setup_vrs(void *hMem, struct udf_disc *disc);
void LvDVDUdf_setup_anchor(void *hMem, struct udf_disc *disc,int DataMode);
void LvDVDUdf_setup_vds(void *hMem, struct udf_disc *disc,uint16_t DataMode, udfinfo_t *pUdfInfo);
void LvDVDUdf_freeUdfdisc(void *hMem, struct udf_disc * disc);
int  LvDVDUdf_insert_desc(void *hMem, struct udf_desc *desc);

#endif//__MKUDFFS_H__
