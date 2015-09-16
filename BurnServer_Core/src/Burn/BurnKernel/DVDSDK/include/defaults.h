
#ifndef __DEFAULTS_H__
#define __DEFAULTS_H__

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

extern LVUDF_DEFAULTSINFO_T LvUDF_Defaults;

#endif//__DEFAULTS_H__
