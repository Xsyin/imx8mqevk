#include <linux/cred.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/mm_types.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/nsproxy.h>
#include <linux/string.h>
#include <linux/list.h> 
#include <linux/secure_container.h>
#include <linux/io.h>
#include <asm/fixmap.h>
#include <asm/sysreg.h>

struct secure_container_region container_region;
static int count;
unsigned long shm_vaddr;

int init_secure_container_region(void)
{
    unsigned long vaddr;
	unsigned long paddr;
	size_t size;
	
    // struct task_struct *p, *ts = &init_task;
	// struct list_head *pos;
	// int count = 0;
    struct gen_pool *genpool = NULL;
    int rc;
    
    shm_vaddr = memremap(0xffc00000, 0x400000, MEMREMAP_WB);

    vaddr = __fix_to_virt(FIX_CONTAINER_BEGIN);
    paddr = (unsigned long)0xfc000000;
    size = SZ_32M;
    container_region.vaddr = vaddr;
    container_region.paddr = paddr;
    container_region.vpoffset = vaddr - paddr;
    container_region.size = size;

    
    pr_info("-------------%s: list tasks offset of tsk: 0x%08x, pid offset: 0x%08x ", __func__, offsetof(struct task_struct, tasks), offsetof(struct task_struct, pid));

    pr_info("<<<<<<<<<< %s: container vaddr : %#016llx, paddr: %#016llx", __func__, container_region.vaddr, container_region.paddr);

    genpool = gen_pool_create(PAGE_SHIFT, -1);
    if (!genpool)
		return -ENOMEM;
    rc = gen_pool_add_virt(genpool, vaddr, paddr, size, -1);
	if (rc) {
		gen_pool_destroy(genpool);
		return rc;
	}

    container_region.container_pool = genpool;

    return 0;
}

// core_initcall(init_secure_container_region);

unsigned long container_pool_alloc(struct gen_pool *genpool, size_t s)
{
    unsigned long va;
	unsigned long con_pva;
	phys_addr_t con_pa;
	int rc;

    va = gen_pool_alloc(genpool, s);
	con_pva = 0;
	if(!va){
		con_pva = gen_pool_alloc(container_region.container_pool, PAGE_SIZE);
        if(!con_pva){
			pr_info("================  %s: gen pool from container pool error !va" , __func__);
            return 0;
		}
		con_pa = gen_pool_virt_to_phys(container_region.container_pool, con_pva);
		rc = gen_pool_add_virt(genpool, con_pva, con_pa, PAGE_SIZE, -1);
		// rc = gen_pool_add(nsproxy_pool, con_pva, PAGE_SIZE, -1);
		if (rc) {
			gen_pool_destroy(genpool);
			pr_info("================  %s: gen pool error" , __func__);
			return 0;
		}
		va = gen_pool_alloc(genpool, s);

		if(!va){
			pr_info("================  %s: gen pool error !va" , __func__);
            return 0;
		}
	}
	memset((void *)va, 0, s);
	con_pa = gen_pool_virt_to_phys(genpool, va);

	pr_info("================  %s: %s vaddr: 0x%lx, con pva: 0x%lx, pa: 0x%lx, s: 0x%08x" , __func__, genpool->name, va, con_pva, con_pa, s);

    return va;
}

struct gen_pool *container_pool_create(const char *name, size_t s)
{
    struct gen_pool *genpool;
    genpool = gen_pool_create(ilog2(s), -1);
    genpool->name = name;

    return genpool;
}

void container_pool_free(struct gen_pool *genpool, void *vaddr, size_t s)
{
	gen_pool_free(genpool, vaddr, s);

}


static void registers_dump(void)
{
    pr_info("#####################################");
    pr_info("# Register SCTLR: 0x%016x  #", read_sysreg_s(SYS_SCTLR_EL1));
    pr_info("# Register MPIDR: 0x%016x #", read_sysreg_s(SYS_MPIDR_EL1));
    pr_info("# Register ESR: 0x%016x #", read_sysreg(esr_el1));
    pr_info("# Register FAR: 0x%016x #", read_sysreg(far_el1));
    pr_info("#####################################");
}

int container_region_update(void)
{
    unsigned long *value;
    struct arm_smccc_res res;
    
    value = (unsigned long *)container_region.vaddr;
    *value = 0x11111111;
    arm_smccc_smc(OPTEE_SMC_UPDATE_CONTAINER_REGION, 0, 0, 0, 0, 0, 0, 0, &res);

    pr_info("<<<<<<<11111111 %s: container start %#016llx, value: %#016llx", __func__, value, *value);

    *value = 0x33333333;
    *(unsigned long *)shm_vaddr = 0x2333beef;
    // registers_dump();
    arm_smccc_smc(OPTEE_SMC_UPDATE_CONTAINER_REGION, 0, 0, 0, 0, 0, 0, 0, &res);
    pr_info("<<<<<<<22222222 %s: container start %#016llx, value: %#016llx", __func__, value, *value);

    pr_info("<<<<<<<33333333 %s: shm addr %#016llx, value: %#016llx", __func__, shm_vaddr, *(unsigned long *)shm_vaddr);
    
    return 0;
}

// int check_coherence(void)
// {
//     struct arm_smccc_res res;

    
//     arm_smccc_smc(OPTEE_SMC_GET_NSPROXY, 0, 0, 0, 0, 0, 0, 0, &res);

//     return 0;
// }

int container_region_free(void)
{
    return 0;
}