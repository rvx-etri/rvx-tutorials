#include "cam_hdmi.h"
#include "hdmi_wdma.h"
#include "hdmi_rdma.h"

extern void timer_start(void);
extern int timer_get_cur_ms(void);

static sHWDMA hw;
static sHRDMA hr;

void cam_hdmi_test(void)
{
	int i, lock;
	int width, height;

	printf("cam_hdmi_test()\n");

	// check HDMI WDMA PLL LOCK
	hdmiwdma_set_hpd(1);
	delay_ms(100);
	
	// reset 이후에 pll lock 체크하고 그 이후에 enable
	hdmiwdma_reset_dvi2rgb(1);
	delay_ms(10);
	hdmiwdma_reset_dvi2rgb(0);
	delay_ms(10);
	hdmiwdma_reset_dvi2rgb(1);
	delay_ms(10);

	for(i=0; i<100; i++) {
		lock = hdmiwmda_read_plllock();
		//printf("pll lock : %d\n", lock);
		delay_ms(100);

		if(lock == 3)
			break;
	}
	printf("PLL LOCKED\n");
	// 

	//REG32(HDMIWDMA_TEST)  = 1;

	// buffer clear
	for(i=0; i<2048*1080*2; i+=4) {
		REG32(0x21000000 + i) = 0x00800080;
	}


	// HDMI WDMA
	width  = 1920;
	height = 1080;
	//width  = 1280;
	//height =  720;
	hw.addr = 0x21000000;

	hdmiwdma_init_param((sHWDMA *)&hw, width, height, HR_COLOR_YCbCr);
	hw.stride = ALIGN1K(1920 * 2);


	hdmiwdma_set_param((sHWDMA *)&hw);

	// HDMI RDMA
	hr.addr0 = 0x21000000;
	hr.addr1 = 0x21000000;
	//hdmirdma_init_param((sHRDMA *)&hr, width, height, HR_COLOR_YCbCr);
	hdmirdma_init_param((sHRDMA *)&hr, 1920, 1080, HR_COLOR_YCbCr);
	hdmirdma_set_param((sHRDMA *)&hr);

	
	//
	hdmiwmda_start();
	hdmirdma_start();

	while(1) {

		if(my_getcnowait() != 0) {
			break;
		}
		delay_ms(10);
	}


	hdmiwmda_stop();
	hdmirdma_stop();
	



	printf("Done\n");
}

