#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>

# board-specific build parameters
BOOT_MODE	:= 	flexspi_nor
BOARD		:=	amyir

 # get SoC common build parameters
include plat/nxp/soc-ls1028/soc.mk

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c

