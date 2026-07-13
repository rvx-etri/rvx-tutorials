#include "ov5640_mp5_regs.h"

void configure_i2c_for_cis_ov5640();
void cis_ov5640_write_i2c(unsigned char id, unsigned int addr, unsigned char data);
void cis_ov5640_write_sensor_vals(sensor_reg reglist[]);
unsigned char cis_ov5640_read_i2c(unsigned int addr);
void cis_ov5640_check_test_reg(void);

