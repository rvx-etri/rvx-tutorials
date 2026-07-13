#include "ervp_assert.h"
#include "vio_util.h"

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
