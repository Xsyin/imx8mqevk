/* SPDX-License-Identifier: GPL-2.0 */

/*
 # created by xsyin 2021-3-19
 # map the secure container region 0xffff 7dff fcc0 0000 ~ 0xffff 7dff fec0 0000
 # get, read, write that region
*/

#ifndef _LINUX_SECURE_CONTAINER_H
#define _LINUX_SECURE_CONTAINER_H

#include <linux/arm-smccc.h>
#include <linux/types.h>
#include <linux/genalloc.h>
// #include <linux/log2.h>

#define OPTEE_SMC_RETURN_OK		0x0

#define OPTEE_SMC_FAST_CALL_VAL(func_num) \
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, ARM_SMCCC_SMC_32, \
			   ARM_SMCCC_OWNER_TRUSTED_OS, (func_num))


#define OPTEE_SMC_FUNCID_UPDATE_CONTAINER_REGION   15
#define OPTEE_SMC_UPDATE_CONTAINER_REGION \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_UPDATE_CONTAINER_REGION)



struct gen_pool;

struct secure_container_region
{
    unsigned long vaddr;
    unsigned long paddr;
	unsigned long vpoffset;
    size_t size;
	struct gen_pool *container_pool;
};

extern struct secure_container_region container_region;

int init_secure_container_region(void);
unsigned long container_pool_alloc(struct gen_pool *genpool, size_t s);
struct gen_pool *container_pool_create(const char *name, size_t s);
void container_pool_free(struct gen_pool *genpool, void *vaddr, size_t s);


int check_coherence(void);
int container_region_update(void);
int container_region_free(void);

#endif