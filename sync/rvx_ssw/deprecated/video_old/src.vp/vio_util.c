#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_multicore_synch.h"
#include "ervp_uart_define.h"

#include "vio_util.h"

void write_reg(unsigned int addr, unsigned int data)
{
        REG32(addr) = data;
}

unsigned int read_reg(unsigned int addr)
{
        return REG32(addr);
}

void putc_02x( int data)
{
	int d;

	data = data & 0xff;

	d = data>>4; // 0 ~ 15
	if(d < 10) uart_putc(0, d + '0');
	else       uart_putc(0, d-10 + 'A');
	d = data&0x0f; // 0 ~ 15
	if(d < 10) uart_putc(0, d + '0');
	else       uart_putc(0, d-10 + 'A');
	uart_putc(0, ' ');
}

void putc_04x( int data)
{
	int d;

	data = data & 0xffff;

	d = data>>12; // 0 ~ 15
	if(d < 10) uart_putc(0, d + '0');
	else       uart_putc(0, d-10 + 'A');
	d = (data>>8)&0x0f; // 0 ~ 15
	if(d < 10) uart_putc(0, d + '0');
	else       uart_putc(0, d-10 + 'A');
	d = (data>>4)&0x0f; // 0 ~ 15
	if(d < 10) uart_putc(0, d + '0');
	else       uart_putc(0, d-10 + 'A');
	d = (data>>0)&0x0f; // 0 ~ 15
	if(d < 10) uart_putc(0, d + '0');
	else       uart_putc(0, d-10 + 'A');

	uart_putc(0, ' ');
}

void vio_clear_buffer(unsigned int addr, unsigned int offset0, unsigned int offset1, unsigned int stride, unsigned int height, int type)
{
	int i;
	int size;

	size = stride * height;

	// buffer clear

	if(type == VIO_FMT_YUVPLANE) {
		for(i=0; i<size; i+=4)   REG32(addr + i) = 0x00000000;
		for(i=0; i<size/4; i+=4) REG32(addr + offset0 + i) = 0x80808080;
		for(i=0; i<size/4; i+=4) REG32(addr + offset1 + i) = 0x80808080;
	}
	else if(type == VIO_FMT_YUVINTRV) {
		for(i=0; i<size; i+=4)   REG32(addr + i) = 0x00000000;
		for(i=0; i<size/2; i+=4) REG32(addr + offset0 + i) = 0x80808080;
	}
	else if(type == VIO_FMT_RGBPLANE) {
		for(i=0; i<size; i+=4)  REG32(addr + i) = 0x00000000;
		for(i=0; i<size; i+=4)	REG32(addr + offset0 + i) = 0x00000000;
		for(i=0; i<size; i+=4)	REG32(addr + offset1 + i) = 0x00000000;
	}
	else if(type == VIO_FMT_RGBINTRV) {
		for(i=0; i<size; i+=4)  REG32(addr + i) = 0x00000000; // <- 이렇게 해야 하는데 코드 실행이 안됨.
		//for(i=0; i<size*2; i+=4)  REG32(addr + i) = 0x00000000; // <- size*2 로 해야 main 코드 실행됨.
		
	}


}
