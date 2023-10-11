#ifndef _MMC_DEF_H_
#define _MMC_DEF_H_

#include "../c906/delay.h"
#include "egos.h"
#include <string.h>

#define RISCV_FENCE(p, s) \
	__asm__ __volatile__ ("fence " #p "," #s : : : "memory")

#define wmb()		RISCV_FENCE(ow, ow)

#define readl(addr)           (*((volatile uint32_t *)(uintptr_t)addr))
#define writel(v, addr)       (*((volatile uint32_t *)(uintptr_t)addr) = (uint32_t)v)

#define SMHC0_BASE 0x04020000
#define SMHC1_BASE 0x04021000
#define SMHC2_BASE 0x04022000   

#define SUNXI_MMC0_BASE	(SMHC0_BASE)
#define SUNXI_MMC1_BASE	(SMHC1_BASE)
#define SUNXI_MMC2_BASE	(SMHC2_BASE)

#define MAX_MMC_NUM	2

#define MMC_TRANS_BY_DMA

#define MMC_REG_FIFO_OS		(0x200U)

#define MMC_REG_BASE		SUNXI_MMC0_BASE
#define CCMU_HCLKGATE0_BASE CCU_SMHC_BGR_REG
#define CCMU_HCLKRST0_BASE 	CCU_SMHC_BGR_REG
#define CCMU_MMC0_CLK_BASE 	CCU_SDMMC0_CLK_REG
#define CCMU_MMC2_CLK_BASE 	CCU_SDMMC2_CLK_REG

#define __be32_to_cpu(x) \
	((uint32_t)( \
		(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) | \
		(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) | \
		(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) | \
		(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24) ))

# define __maybe_unused	

void invalidate_dcache_range(__maybe_unused unsigned long start,
			     __maybe_unused unsigned long stop);
void flush_dcache_range(__maybe_unused unsigned long start,
			__maybe_unused unsigned long stop);

#define __mmc_be32_to_cpu(x)	((0x000000ff&((x)>>24)) | (0x0000ff00&((x)>>8)) | 			\
							 (0x00ff0000&((x)<<8)) | (0xff000000&((x)<<24)))

#define IOMEM_ADDR(addr) ((volatile void *)((uintptr_t)(addr)))
#define PT_TO_U32(p)   ((uint32_t)((uintptr_t)(p)))
#define PT_TO_U(p)   ((uintptr_t)(p))
#define WR_MB() 	wmb()

#define  OSAL_CacheRangeFlush(__s, __l, __a)  flush_dcache_range(PT_TO_U(__s), PT_TO_U(__s)+__l - 1)
#define  OSAL_CacheRangeInvaild(__s, __l, __a)  invalidate_dcache_range(PT_TO_U(__s), PT_TO_U(__s)+__l - 1)

#ifndef NULL
#define NULL (void *)0
#endif

/*#define MMC_DEBUG*/
#ifdef MMC_DEBUG
#define mmcinfo(fmt...)	INFO("[mmc]: "fmt)
#define mmcdbg(fmt...)	INFO("[mmc]: "fmt)
#define mmcmsg(fmt...)	INFO(fmt)
#else
#define mmcinfo(fmt...)	INFO("[mmc]: "fmt)
#define mmcdbg(fmt...)
#define mmcmsg(fmt...)
#endif

#define DMAC_DES_BASE_IN_SRAM		(0x20000 + 0xC000)
#define DMAC_DES_BASE_IN_SDRAM		(0x42000000)
#define DRAM_START_ADDR				(0x40000000)

#define DRIVER_VER  "2021-04-2 16:45"

#endif				/* _MMC_H_ */
