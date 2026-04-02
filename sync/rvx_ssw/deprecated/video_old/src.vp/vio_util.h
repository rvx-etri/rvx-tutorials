#ifndef _H_VIO_UTIL_H_
#define _H_VIO_UTIL_H_

typedef enum {
        VIO_FMT_YUVPLANE = 0,
        VIO_FMT_YUVINTRV = 1,
        VIO_FMT_RGBPLANE = 2,
        VIO_FMT_RGBINTRV = 3
} eVIO_FMT;

void write_reg(unsigned int addr, unsigned int data);
unsigned int read_reg(unsigned int addr);

void putc_02x(int data);
void putc_04x(int data);

void vio_clear_buffer(unsigned int addr, unsigned int offset0, unsigned int offset1, unsigned int stride, unsigned int height, int type);

#endif
