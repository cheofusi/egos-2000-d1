/*
 * (C) Copyright 2013-2016
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
#ifndef _MMC_BSP_H_
#define _MMC_BSP_H_

#include <stdint.h>

struct sunxi_mmc_des {
	uint32_t:1, dic:1,		/* disable interrupt on completion */
	last_des:1,		/* 1-this data buffer is the last buffer */
	first_des:1,		/* 1-data buffer is the first buffer,
				   0-data buffer contained in the next descriptor is 1st buffer */
	des_chain:1,		/* 1-the 2nd address in the descriptor is the next descriptor address */
	end_of_ring:1,		/* 1-last descriptor flag when using dual data buffer in descriptor */
	: 24,
	card_err_sum:1,	/* transfer error flag */
	own:1;			/* des owner:1-idma owns it, 0-host owns it */

#define SDXC_DES_NUM_SHIFT 12
#define SDXC_DES_BUFFER_MAX_LEN	(1 << SDXC_DES_NUM_SHIFT)
	uint32_t data_buf1_sz:16, data_buf2_sz:16;

	uint32_t buf_addr_ptr1;
	uint32_t buf_addr_ptr2;
};

struct sunxi_mmc_host {
	struct sunxi_mmc *reg;
	uint32_t mmc_no;
	/*uint32_t  mclk;*/
	uint32_t hclkrst;
	uint32_t hclkbase;
	uint32_t mclkbase;
	uint32_t database;
	uint32_t commreg;
	uint32_t fatal_err;
	struct sunxi_mmc_des *pdes;

	/*sample delay and output deley setting */
	uint32_t timing_mode;
	struct mmc *mmc;
	uint32_t mod_clk;
	uint32_t clock;

};

struct mmc_cmd {
	unsigned cmdidx;
	unsigned resp_type;
	unsigned cmdarg;
	unsigned response[4];
	unsigned flags;
};

struct mmc_data {
	union {
		char *dest;
		const char *src;/* src buffers don't get written to */
	} b;
	unsigned flags;
	unsigned blocks;
	unsigned blocksize;
};

struct tuning_sdly {
	/*uint8_t sdly_400k;*/
	uint8_t sdly_25M;
	uint8_t sdly_50M;
	uint8_t sdly_100M;
	uint8_t sdly_200M;
};/*size can not over 256 now*/

/* speed mode */
#define DS26_SDR12            (0)
#define HSSDR52_SDR25         (1)
#define HSDDR52_DDR50         (2)
#define HS200_SDR104          (3)
#define HS400                 (4)
#define MAX_SPD_MD_NUM        (5)

/* frequency point */
#define CLK_400K         (0)
#define CLK_25M          (1)
#define CLK_50M          (2)
#define CLK_100M         (3)
#define CLK_150M         (4)
#define CLK_200M         (5)
#define MAX_CLK_FREQ_NUM (8)

/*
	timing mode
	1: output and input are both based on phase
	3: output is based on phase, input is based on delay chain
	4: output is based on phase, input is based on delay chain.
	    it also support to use delay chain on data strobe signal.
*/
#define SUNXI_MMC_TIMING_MODE_1 1U
#define SUNXI_MMC_TIMING_MODE_3 3U
#define SUNXI_MMC_TIMING_MODE_4 4U

extern void mmc_update_host_caps_r(int sdc_no);

#endif				/* _MMC_BSP_H_ */
