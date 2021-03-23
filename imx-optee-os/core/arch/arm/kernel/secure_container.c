#include <kernel/secure_container.h>
#include <kernel/panic.h>
#include <mm/tee_mm.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <sm/optee_smc.h>
#include <platform_config.h>
#include <initcall.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <drivers/tzc380.h>
#include <platform_config.h>
#include <arm64.h>
#include <io.h>

static void registers_dump(void)
{

    DMSG("Register esr: 0x%" PRIx64, read_esr_el1());
	DMSG("Register tcr: 0x%" PRIx64, read_tcr_el1());
	DMSG("Register sctlr: 0x%" PRIx32, read_sctlr_el1());
	DMSG("Register far: 0x%" PRIx64, read_far_el1());
	DMSG("Register MPIDR: 0x%" PRIx64, read_mpidr_el1());
	DMSG("Register TTBR1: 0x%" PRIx64, read_ttbr1_el1());
}


void tee_entry_update_container_region(struct thread_smc_args *args)
{
	vaddr_t start_addr, end_addr;
	uint64_t *start_value, *end_value;

	core_mmu_get_mem_by_type(MEM_AREA_TEE_CONTAINER, &start_addr, &end_addr);
	
	start_value = (uint64_t *)start_addr;

	DMSG("container region: start_addr 0x%" PRIxPA ", start_value 0x%" PRIxPA, start_addr, *start_value);

    // registers_dump();
	*start_value = 0x22222222;
	

	DMSG("container region: start_addr 0x%" PRIxPA ", start_value 0x%" PRIxPA, start_addr, *start_value);

	core_mmu_get_mem_by_type(MEM_AREA_NSEC_SHM, &start_addr, &end_addr);
	start_value = (uint64_t *)start_addr;
	DMSG("container region: NSEC_SHM 0x%" PRIxPA ", value 0x%" PRIxPA, start_addr, *start_value);
	*start_value = 0xdeadbeef;
	DMSG("container region: NSEC_SHM 0x%" PRIxPA ", value 0x%" PRIxPA, start_addr, *start_value);


	args->a0 = OPTEE_SMC_RETURN_OK;
	args->a1 = 0;
	args->a2 = 0;
	args->a3 = 0;
}


// void update_container_handle(struct sm_ctx *ctx, unsigned long status, unsigned long data_paddr, unsigned long instr_paddr)
// {
//     DMSG("update_container_handle: NS ctx lr 0x%lx ", ctx->nsec.r0);
//     DMSG("update_container_handle: status 0x%lx, data_paddr 0x%lx, instr_paddr 0x%lx", status, data_paddr, instr_paddr);
// }

void update_container_handle(void)
{
    DMSG("update_container_handle: NS ctx lr ");
    DMSG("update_container_handle: status ");
}