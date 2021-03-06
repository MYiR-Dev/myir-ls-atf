/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <debug.h>
#include <errno.h>
#include <string.h>
#include <io.h>
#include <ddr.h>
#include <utils.h>
#include <utils_def.h>
#include <errata.h>
#include <sfp.h>
#include <delay_timer.h>
#ifdef CONFIG_STATIC_DDR
const struct ddr_cfg_regs static_1600 = {
	.cs[0].config = 0x80040422,
	.cs[0].bnds = 0xFF,
	.sdram_cfg[0] = 0xC50C0004,
	.sdram_cfg[1] = 0x401100,
	.timing_cfg[0] = 0x91550018,
	.timing_cfg[1] = 0xBAB40C42,
	.timing_cfg[2] = 0x48C111,
	.timing_cfg[3] = 0x1111000,
	.timing_cfg[4] = 0x2,
	.timing_cfg[5] = 0x3401400,
	.timing_cfg[7] = 0x23300000,
	.timing_cfg[8] = 0x2114600,
	.sdram_mode[0] = 0x3010210,
	.sdram_mode[9] = 0x4000000,
	.sdram_mode[8] = 0x500,
	.sdram_mode[2] = 0x10210,
	.sdram_mode[10] = 0x400,
	.sdram_mode[11] = 0x4000000,
	.sdram_mode[4] = 0x10210,
	.sdram_mode[12] = 0x400,
	.sdram_mode[13] = 0x4000000,
	.sdram_mode[6] = 0x10210,
	.sdram_mode[14] = 0x400,
	.sdram_mode[15] = 0x4000000,
	.interval = 0x18600618,
	.data_init = 0xdeadbeef,
	.zq_cntl = 0x8A090705,
	.clk_cntl = 0x2000000,
	.cdr[0] = 0x80040000,
	.cdr[1] = 0xA181,
	.wrlvl_cntl[0] = 0x8675F605,
	.wrlvl_cntl[1] = 0x6070700,
	.wrlvl_cntl[2] = 0x0000008,
	.dq_map[0] = 0x5b65b658,
	.dq_map[1] = 0xd96d8000,
	.dq_map[2] = 0,
	.dq_map[3] = 0x1600000,
	.debug[28] = 0x00700046,
};

long long board_static_ddr(struct ddr_info *priv)
{
	memcpy(&priv->ddr_reg, &static_1600, sizeof(static_1600));

	return 0x100000000ULL;
}

#else
static const struct rc_timing rcz[] = {
	{1600, 8, 7},
	{}
};

static const struct board_timing ram[] = {
        {0x1f, rcz, 0x01010101, 0x01010101},
};

int ddr_board_options(struct ddr_info *priv)
{
	int ret;
	struct memctl_opt *popts = &priv->opt;

	ret = cal_board_params(priv, ram, ARRAY_SIZE(ram));
	if (ret)
		return ret;

	popts->bstopre = 0x40; /* precharge value */
	popts->half_strength_drive_en = 1;
	popts->cpo_sample = 0x46;
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN |
			  DDR_CDR1_ODT(DDR_CDR_ODT_50ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_50ohm) |
			  DDR_CDR2_VREF_OVRD(70);	/* Vref = 70% */

	popts->addr_hash = 1; /* address hashing */
	return 0;
}

/* DDR model number:  MT40A512M16LY-062:E */
struct dimm_params ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 2147483648u,
	.capacity = 2147483648u,
	.primary_sdram_width = 32,
	.ec_sdram_width = 4,
	.rdimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 16,
	.n_col_addr = 10,
	.bank_group_bits = 1,
	.edc_config = 0,
	.burst_lengths_bitmask = 0x0c,  /* MYIR ADD:need check*/
	.tckmin_x_ps = 750,
	.tckmax_ps = 1900,
	.caslat_x = 0x0001FFE00, /* MYIR ADD: need check*/
	.taa_ps = 13500,
	.trcd_ps = 13500,
	.trp_ps = 13500,
	.tras_ps = 33000,
	.trc_ps = 46500,
	.twr_ps = 15000,
	.trfc1_ps = 350000,
	.trfc2_ps = 260000,
	.trfc4_ps = 160000,
	.tfaw_ps = 30000,
	.trrds_ps = 5300,/* MYIR ADD: need check*/
	.trrdl_ps = 6400,/* MYIR ADD: need check*/
	.tccdl_ps = 5355,/* MYIR ADD: need check*/
	.refresh_rate_ps = 7800000,
        .dq_mapping[0] = 0x0,
        .dq_mapping[1] = 0x0,
        .dq_mapping[2] = 0x0,
        .dq_mapping[3] = 0x0,
        .dq_mapping[4] = 0x0,
	.dq_mapping_ors = 0,
	.rc = 0x1f,
};

int ddr_get_ddr_params(struct dimm_params *pdimm,
			    struct ddr_conf *conf)
{
	static const char dimm_model[] = "Fixed DDR on board";

	conf->dimm_in_use[0] = 1;
	memcpy(pdimm, &ddr_raw_timing, sizeof(struct dimm_params));
	memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);

	return 1;
}
#endif
#define GPDIR_REG_OFFSET 0x0
#define GPDAT_REG_OFFSET 0x8
#define GPIBE_REG_OFFSET 0x18
void setup_uart_control_pin(void)
{

	uint8_t *gpdir = NULL;
	uint8_t *gpdat = NULL;
	uint8_t *gpibe = NULL;
	uint32_t val = 0;
	uint32_t bit_num = 1 << (31-25);

	gpdir = (uint8_t *)NXP_GPIO1_ADDR + GPDIR_REG_OFFSET;
	gpdat = (uint8_t *)NXP_GPIO1_ADDR + GPDAT_REG_OFFSET;
	gpibe = (uint8_t *)NXP_GPIO1_ADDR + GPIBE_REG_OFFSET;
	
	/*set ibe*/
	val = 0xffffffff;
	sfp_write32(gpibe, val);
	val = sfp_read32(gpibe);

	/*set dir*/
	val = sfp_read32(gpdir);
	val = val | bit_num;
	sfp_write32(gpdir, val);
	
	val = sfp_read32(gpdir);
	if (!(val & bit_num))
		printf("Configure Pin Dir Error!\n");
		
	mdelay(1);
	
	/*set data*/
	val = sfp_read32(gpdat);
	val = val | bit_num;
	sfp_write32(gpdat, val);
	
	val = sfp_read32(gpdat);
	if (!(val & bit_num))
		printf("Configure Pin Data Error!\n");

}
long long _init_ddr(void)
{
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;
	
	setup_uart_control_pin();
	
	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);

	zeromem(&info, sizeof(struct ddr_info));
	info.num_ctlrs = 1;
	info.dimm_on_ctlr = 1;
	info.clk = get_ddr_freq(&sys, 0);
	info.ddr[0] = (void *)NXP_DDR_ADDR;

	dram_size = dram_init(&info);

	if (dram_size < 0)
		ERROR("DDR init failed.\n");

	return dram_size;
}
