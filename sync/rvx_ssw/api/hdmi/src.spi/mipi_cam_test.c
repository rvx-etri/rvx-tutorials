#include "platform_info.h"
#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_multicore_synch.h"
#include "ervp_external_peri_group_memorymap_offset.h"
#include "frvp_spi.h"
#include "ervp_uart_define.h"
#include "ervp_reg_util.h"
#include "ervp_misc_util.h"
#include "types.h"

//#include "orvp_i2c.h"
//#include "camera.h"
//#include "ov5642_regs.h"
#include "cam_spi.h"
#include "hdmi_rdma.h"
#include "cam_mipi.h"
#include "mipi_wdma.h"


static sHRDMA hr;
static sMWDMA mw;


//////////////////////////////////////////////////////////////////////
// pcam ���� �ڵ� �����Ͽ� �׽�Ʈ

void mipi_cam_zybo_demo_test(void)
{
	int i;

	printf(" mipi_cam_zybo_demo_test \n");

	REG32(MIPIWDMA_TEST) = 1; // !!!!
	delay_ms(100);


	// PS_IIC<ScuGicInterruptController> iic_driver(CAM_I2C_DEVID, irpt_ctl, CAM_I2C_IRPT_ID, 100000);
	configure_i2c(I2C_INDEX_FOR_MIPI_CAM, I2C_FREQ_OF_MIPI_CAM, I2C_ENABLE, I2C_INTERRUPT_DISABLE);


	// OV5640 cam(iic_driver, gpio_driver);
	ov5640_cam();

	// VideoOutput vid(XPAR_VTC_0_DEVICE_ID, XPAR_VIDEO_DYNCLK_DEVICE_ID);
	printf("hdmi init\n");
	hdmi_tx_init();
	REG32(HDMIRDMA_EN		) = 0xffffffff; // HDMI TX On
	hdmirdma_set_idx(0);


	printf("mipi init\n");
	mipi_tx_init();

	// pipeline_mode_change(vdma_driver, cam, vid, Resolution::R1920_1080_60_PP, OV5640_cfg::mode_t::MODE_1080P_1920_1080_30fps);
	//Bring up input pipeline back-to-front
	{
		//vdma_driver.resetWrite();
		//MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
		REG32(MIPIWDMA_AXI1	) = 1; // MIPI_CSI_Reset
		//MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
		REG32(MIPIWDMA_AXI0	) = 1; // MIPI_DPHY_Reset
		//cam.reset();
		ov5640_reset();
	}

	{
		//vdma_driver.configureWrite(timing[static_cast<int>(res)].h_active, timing[static_cast<int>(res)].v_active);
		//Xil_Out32(GAMMA_BASE_ADDR, 3); // Set Gamma correction factor to 1/1.8
		//TODO CSI-2, D-PHY config here
		//cam.init();
		ov5640_init();
	}

	{
		//vdma_driver.enableWrite();
		//MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, CR_ENABLE_MASK);
		REG32(MIPIWDMA_AXI1	) = 2; // MIPI_CSI_Enable
		//MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, CR_ENABLE_MASK);
		REG32(MIPIWDMA_AXI0	) = 2; // MIPI_DPHY_Enable
		//cam.set_mode(mode);
		//cam.set_awb(OV5640_cfg::awb_t::AWB_ADVANCED);
		cam_mipi_config_sensor(OV5640_1080P_15FPS);

		printf("sensor cfg write done\n");

	}

	REG32(MIPIWDMA_EN		) = 0xffffffff; // MIPI TX On


	//Bring up output pipeline back-to-front
	{
		//vid.reset();
		//vdma_driver.resetRead();
	}

	{
		//vid.configure(res);
		//vdma_driver.configureRead(timing[static_cast<int>(res)].h_active, timing[static_cast<int>(res)].v_active);
	}

	{
		//vid.enable();
		//vdma_driver.enableRead();
		//REG32(HDMIRDMA_EN		) = 0xffffffff; // HDMI TX On
		//hdmirdma_set_idx(0);

	}


	timer_start();
	for(i=0; i<2000; i++) {
		
		int reg50, reg54, reg40, reg1c, reg20;

		reg40 = REG32(MIPIWDMA_STATUS);
		reg50 = REG32(MIPIWDMA_DBG50);
		reg54 = REG32(MIPIWDMA_DBG54);

		reg1c = REG32(MIPIWDMA_AXI0);
		reg20 = REG32(MIPIWDMA_AXI1);

		//printf("%5d %08x %08x %08x  %08x  %08x\n", i, reg40, reg50, reg54, reg1c, reg20);
		printf("%5d %08x %08x %08x  %8d\n", i, reg40, reg50, reg54, timer_get_cur_ms());
		

		REG32(MIPIWDMA_STATUS) = 1;

		delay_ms(1);
	}

	my_getc();

	REG32(HDMIRDMA_EN		) = 0;
	REG32(MIPIWDMA_EN		) = 0;

}



void ov5640_cam(void)
{
	ov5640_reset();
	ov5640_init();
}

void ov5640_reset(void)
{
	mipiwdma_set_sensor_pwdn(0);
	delay_ms(1000);
	mipiwdma_set_sensor_pwdn(1);
	delay_ms(1000);
}

void ov5640_init(void)
{
	uint8_t id_h, id_l;

	id_h = cam_mipi_read_i2c(OV5640_CHIPID_HIGH);
	id_l = cam_mipi_read_i2c(OV5640_CHIPID_LOW);
	if ((id_h != 0x56) || (id_l != 0x40)) {
		printf("ov5640_init : Got %02x %02x. Expected %02x %02x\r\n", id_h, id_l, 0x56, 0x40);
	}

	//[1]=0 System input clock from pad; Default read = 0x11
	cam_mipi_write_i2c(0x3103, 0x11);
	//[7]=1 Software reset; [6]=0 Software power down; Default=0x02
	cam_mipi_write_i2c(0x3008, 0x82);
	delay_ms(1000);

	cam_mipi_write_sensor_vals(OV5640_cfg_init_);

	//Stay in power down
}


void hdmi_tx_init(void)
{
	int o_width, o_height;

	o_width  = 1920; o_height = 1080; // hdmi fhd


	// HDMI TX �ʱ�ȭ
	hr.addr0 = 0x21000000;
	hr.addr1 = 0x21000000;

	hdmirdma_init_param((sHRDMA *)&hr, o_width, o_height, HR_COLOR_YCbCr);
	hdmirdma_set_param((sHRDMA *)&hr);

	memset(hr.addr0, 0x80, hr.stride*hr.vsize);
	memset(hr.addr1, 0x80, hr.stride*hr.vsize);
	//for(i=0; i<hr.stride*hr.vsize; i+=4) {	REG32(0x21000000 + i) = 0x80808080;	}
	//for(i=0; i<hr.stride*hr.vsize; i+=4) {	REG32(0x21800000 + i) = 0x80808080;	}
}

void mipi_tx_init(void)
{
	int i_width, i_height, i_type;

	i_width  = 1920; i_height = 1080; i_type = HR_COLOR_YCbCr; // 

	mw.addr = 0x21000000;
	mipiwdma_init_param((sMWDMA *)&mw, i_width, i_height, i_type);

	// mipi tx stride�� hdmi rx stride�� �°� ����
	if(mw.type == HR_COLOR_ARGB) mw.stride = ALIGN1K(1920 * 4); // 1920 => o_width
	else 		                 mw.stride = ALIGN1K(1920 * 2);
	mipiwdma_set_param((sMWDMA *)&mw);

}