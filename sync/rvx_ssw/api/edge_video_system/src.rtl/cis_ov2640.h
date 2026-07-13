#include "ov2640_regs.h"

void configure_i2c_for_cis_ov2640();
void cis_ov2640_write_i2c(unsigned char id, unsigned char addr, unsigned char data);
void cis_ov2640_write_sensor_vals(sensor_reg reglist[]);
unsigned int cis_ov2640_read_i2c(unsigned int id, unsigned int addr);
void cis_ov2640_check_test_reg(void);

