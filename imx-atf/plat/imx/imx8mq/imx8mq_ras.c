#include <ras.h>

struct ras_interrupt imx8mq_ras_interrupts[] = {
};

struct err_record_info imx8mq_err_records[] = {
};

REGISTER_ERR_RECORD_INFO(imx8mq_err_records);
REGISTER_RAS_INTERRUPTS(imx8mq_ras_interrupts);