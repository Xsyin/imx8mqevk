// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 * Copyright (c) 2016, Wind River Systems.
 * All rights reserved.
 * Copyright 2018 NXP
 */

#include <arm.h>
#include <console.h>
#include <drivers/gic.h>
#ifdef CFG_IMX_LPUART
#include <drivers/imx_lpuart.h>
#else
#include <drivers/imx_uart.h>
#endif
#include <drivers/imx_snvs.h>
#include <io.h>
#include <imx.h>
#include <kernel/cache_helpers.h>
#include <kernel/generic_boot.h>
#include <kernel/misc.h>
#include <kernel/panic.h>
#include <kernel/pm_stubs.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <stdint.h>
#include <sm/optee_smc.h>
#include <tee/entry_fast.h>
#include <tee/entry_std.h>


static void main_fiq(void);
static struct gic_data gic_data;

static const struct thread_handlers handlers = {
	.std_smc = tee_entry_std,
	.fast_smc = tee_entry_fast,
	.nintr = main_fiq,
#if defined(CFG_WITH_ARM_TRUSTED_FW)
	.cpu_on = cpu_on_handler,
	.cpu_off = pm_do_nothing,
	.cpu_suspend = pm_do_nothing,
	.cpu_resume = pm_do_nothing,
	.system_off = pm_do_nothing,
	.system_reset = pm_do_nothing,
#else
	.cpu_on = pm_panic,
	.cpu_off = pm_panic,
	.cpu_suspend = pm_panic,
	.cpu_resume = pm_panic,
	.system_off = pm_panic,
	.system_reset = pm_panic,
#endif
};

#ifdef CONSOLE_UART_BASE
#ifdef CFG_IMX_LPUART
static struct imx_lpuart_data console_data;
#else
static struct imx_uart_data console_data;
#endif

register_phys_mem(MEM_AREA_IO_NSEC, CONSOLE_UART_BASE, CORE_MMU_DEVICE_SIZE);
#endif
#ifdef IMX_OCOTP_BASE
register_phys_mem(MEM_AREA_IO_SEC, IMX_OCOTP_BASE, CORE_MMU_DEVICE_SIZE);
#endif
#ifdef GIC_BASE
register_phys_mem(MEM_AREA_IO_SEC, GIC_BASE, CORE_MMU_DEVICE_SIZE);
#endif
#ifdef ANATOP_BASE
register_phys_mem(MEM_AREA_IO_SEC, ANATOP_BASE, CORE_MMU_DEVICE_SIZE);
#endif
#ifdef GICD_BASE
register_phys_mem(MEM_AREA_IO_SEC, GICD_BASE, 0x10000);
#endif
#ifdef GICR_BASE
register_phys_mem(MEM_AREA_IO_SEC, GICR_BASE, 0xC0000);
#endif
#ifdef AIPS0_BASE
register_phys_mem(MEM_AREA_IO_SEC, AIPS0_BASE,
		  ROUNDUP(AIPS0_SIZE, CORE_MMU_DEVICE_SIZE));
#endif
#ifdef AIPS1_BASE
register_phys_mem(MEM_AREA_IO_SEC, AIPS1_BASE,
		  ROUNDUP(AIPS1_SIZE, CORE_MMU_DEVICE_SIZE));
#endif
#ifdef AIPS2_BASE
register_phys_mem(MEM_AREA_IO_SEC, AIPS2_BASE,
		  ROUNDUP(AIPS2_SIZE, CORE_MMU_DEVICE_SIZE));
#endif
#ifdef AIPS3_BASE
register_phys_mem(MEM_AREA_IO_SEC, AIPS3_BASE,
		  ROUNDUP(AIPS3_SIZE, CORE_MMU_DEVICE_SIZE));
#endif
#ifdef IRAM_BASE
register_phys_mem(MEM_AREA_TEE_COHERENT,
		  ROUNDDOWN(IRAM_BASE, CORE_MMU_DEVICE_SIZE),
		  CORE_MMU_DEVICE_SIZE);
#endif
#ifdef M4_AIPS_BASE
register_phys_mem(MEM_AREA_IO_SEC, M4_AIPS_BASE, M4_AIPS_SIZE);
#endif
#ifdef IRAM_S_BASE
register_phys_mem(MEM_AREA_TEE_COHERENT,
		  ROUNDDOWN(IRAM_S_BASE, CORE_MMU_DEVICE_SIZE),
		  CORE_MMU_DEVICE_SIZE);
#endif

#if defined(CFG_PL310)
register_phys_mem(MEM_AREA_IO_SEC,
		  ROUNDDOWN(PL310_BASE, CORE_MMU_DEVICE_SIZE),
		  CORE_MMU_DEVICE_SIZE);
#endif

/*
 * Mapping needed for dynamic shared memory with the non-secure world.
 * Currently, only the shared memory is supported by u-boot optee driver.
 * All the non-secure RAM can be mapped as dynamic shared memory by a CA
 */
#ifdef DRAM0_NSEC_BASE
register_dynamic_shm(DRAM0_NSEC_BASE, DRAM0_NSEC_SIZE);
#endif

const struct thread_handlers *generic_boot_get_handlers(void)
{
	return &handlers;
}

static void main_fiq(void)
{
	gic_it_handle(&gic_data);
}

void console_init(void)
{

#if defined (CONSOLE_UART_BASE)
#ifdef CFG_IMX_LPUART
	imx_lpuart_init(&console_data, CONSOLE_UART_BASE);
#else
	imx_uart_init(&console_data, CONSOLE_UART_BASE);
#endif
	register_serial_console(&console_data.chip);
#else
	register_serial_console(NULL);
#endif
}

void main_init_gic(void)
{
#ifdef DRAM0_NSEC_BASE
	DMSG("DRAM0_NSEC_BASE: 0x%08" PRIxVA " DRAM0_NSEC_SIZE:0x%08" PRIxPASZ, DRAM0_NSEC_BASE, DRAM0_NSEC_SIZE);
#endif
#ifdef CFG_ARM_GICV3
	vaddr_t gicd_base;

	gicd_base = core_mmu_get_va(GICD_BASE, MEM_AREA_IO_SEC);

	if (!gicd_base)
		panic();

	/* Initialize GIC */
	gic_init(&gic_data, 0, gicd_base);
	itr_init(&gic_data.chip);
#else
	vaddr_t gicc_base;
	vaddr_t gicd_base;

	gicc_base = core_mmu_get_va(GIC_BASE + GICC_OFFSET, MEM_AREA_IO_SEC);
	gicd_base = core_mmu_get_va(GIC_BASE + GICD_OFFSET, MEM_AREA_IO_SEC);

	if (!gicc_base || !gicd_base)
		panic();

	/* Initialize GIC */
	gic_init(&gic_data, gicc_base, gicd_base);
	itr_init(&gic_data.chip);
#endif
}

#if CFG_TEE_CORE_NB_CORE > 1
void main_secondary_init_gic(void)
{
	gic_cpu_init(&gic_data);
}
#endif

#if defined(CFG_BOOT_SYNC_CPU)
void psci_boot_allcpus(void)
{
	vaddr_t src_base = core_mmu_get_va(SRC_BASE, MEM_AREA_TEE_COHERENT);
	uint32_t pa = virt_to_phys((void *)TEE_TEXT_VA_START);

	/* set secondary entry address and release core */
	write32(pa, src_base + SRC_GPR1 + 8);
	write32(pa, src_base + SRC_GPR1 + 16);
	write32(pa, src_base + SRC_GPR1 + 24);

	write32(BM_SRC_SCR_CPU_ENABLE_ALL, src_base + SRC_SCR);
}
#endif

/*
 * Platform CPU reset late function executed with MMU
 * OFF. The CSU must be initialized here to allow
 * access to Non-Secure Memory from Secure code without
 * aborting
 */
void plat_cpu_reset_late(void)
{
	if (get_core_pos() == 0) {
#if defined(CFG_BOOT_SYNC_CPU)
		psci_boot_allcpus();
#endif

#ifdef CFG_SCU
		scu_init();
#endif

#ifdef CFG_CSU
		csu_init();
#endif

#ifdef CFG_TZC380
		tzasc_init();
#endif
	}
}

#ifdef CFG_PSCI_ARM32
/*
 * Platform Wakeup late function executed with MMU
 * ON after suspend.
 */
void plat_cpu_wakeup_late(void)
{
#ifdef CFG_SCU
	scu_init();
	dcache_op_all(DCACHE_OP_CLEAN_INV);
#endif

#ifdef CFG_TZC380
	tzasc_init();
#endif
}
#endif

static TEE_Result platform_security_config(void)
{
#ifdef CFG_IMX_SNVS
	snvs_set_npswa_en();
#endif
	return TEE_SUCCESS;
}

service_init(platform_security_config);
