/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __NAND_PHYSIC_INTERFACE_H__
#define __NAND_PHYSIC_INTERFACE_H__

#include "../aw_nand_type.h"
//#include "nand-partition/phy.h"
#include <sunxi_nand_partitions.h>
extern __u32 storage_type;

extern struct _nand_info aw_nand_info;

/* current logical write block */
extern unsigned short cur_w_lb_no;
/* current physical write block */
extern unsigned short cur_w_pb_no;
/* current physical write page */
extern unsigned short cur_w_p_no;
/* current logical erase block */
extern unsigned short cur_e_lb_no;

__u32 get_storage_type_from_init(void);
struct _nand_info *NandHwInit(void);
int NandHwExit(void);
int NandHwSuperStandby(void);
int NandHwSuperResume(void);
int NandHwNormalStandby(void);
int NandHwNormalResume(void);
int NandHwShutDown(void);
//void *NAND_GetIOBaseAddr(u32 no);
u32 NAND_GetLogicPageSize(void);
int nand_physic_erase_block(unsigned int chip, unsigned int block);
int nand_physic_read_page(unsigned int chip, unsigned int block, unsigned int page, unsigned int bitmap, unsigned char *mbuf, unsigned char *sbuf);
int nand_physic_write_page(unsigned int chip, unsigned int block, unsigned int page, unsigned int bitmap, unsigned char *mbuf, unsigned char *sbuf);
int nand_physic_bad_block_check(unsigned int chip, unsigned int block);
int nand_physic_bad_block_mark(unsigned int chip, unsigned int block);
int nand_physic_block_copy(unsigned int chip_s, unsigned int block_s, unsigned int chip_d, unsigned int block_d, unsigned int copy_nums);
int read_virtual_page(unsigned int nDieNum, unsigned int nBlkNum, unsigned int nPage, uint64 SectBitmap, void *pBuf, void *pSpare);
int write_virtual_page(unsigned int nDieNum, unsigned int nBlkNum, unsigned int nPage, uint64 SectBitmap, void *pBuf, void *pSpare);
int erase_virtual_block(unsigned int nDieNum, unsigned int nBlkNum);
int virtual_badblock_check(unsigned int nDieNum, unsigned int nBlkNum);
int virtual_badblock_mark(unsigned int nDieNum, unsigned int nBlkNum);
__u32 nand_get_lsb_block_size(void);
__u32 nand_get_lsb_pages(void);
__u32 nand_get_phy_block_size(void);
__u32 nand_used_lsb_pages(void);
u32 nand_get_pageno(u32 lsb_page_no);
__u32 nand_get_page_cnt_per_block(void);
__u32 nand_get_pae_size(void);
__u32 nand_get_twoplane_flag(void);
void *nand_get_channel_base_addr(u32 no);
#endif /*NAND_PHYSIC_INTERFACE_H*/
