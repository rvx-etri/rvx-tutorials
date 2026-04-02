#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "ervp_uart_define.h"

#include "platform_info.h"

/* UART 16550 driver */

#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

#define WAIT_FOR_XMITR \
        do { \
                lsr = REG8(UART_BASEADDR + UART_LSR); \
        } while ((lsr & BOTH_EMPTY) != BOTH_EMPTY)

#define WAIT_FOR_THRE \
        do { \
                lsr = REG8(UART_BASEADDR + UART_LSR); \
        } while ((lsr & UART_LSR_THRE) != UART_LSR_THRE)

#define CHECK_FOR_CHAR (REG8(UART_BASEADDR + UART_LSR) & UART_LSR_DR)

#define WAIT_FOR_CHAR \
         do { \
                lsr = REG8(UART_BASEADDR + UART_LSR); \
         } while ((lsr & UART_LSR_DR) != UART_LSR_DR)

void uart_init(void)
{
	int divisor;

	/* Reset receiver and transmiter */
#if 1
	REG8(UART_BASEADDR + UART_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14;	// UART 16550
#else
	//REG8(UART_BASEADDR + UART_FCR) = 0x00;	// UART 8250
#endif

	/* Disable all interrupts */
	REG8(UART_BASEADDR + UART_IER) = 0x00;

	/* Set 8 bit char, 1 stop bit, no parity */
	REG8(UART_BASEADDR + UART_LCR) = UART_LCR_WLEN8 & ~(UART_LCR_STOP | UART_LCR_PARITY);

	/* Set baud rate */
	// System clock 50 MHz, 115200 baud rate
	divisor = UART_CLK_PERIOD/(16 * UART_BAUD_RATE);
	REG8(UART_BASEADDR + UART_LCR) |= UART_LCR_DLAB;
	REG8(UART_BASEADDR + UART_DLL) = divisor & 0x000000ff;
	REG8(UART_BASEADDR + UART_DLM) = (divisor >> 8) & 0x000000ff;
	REG8(UART_BASEADDR + UART_LCR) &= ~(UART_LCR_DLAB);

	/* enable */
	REG8(UART_BASEADDR + UART_EN) = 1;
}

int uart_putc(char c)
{
	unsigned char lsr;

	WAIT_FOR_THRE;
	REG8(UART_BASEADDR + UART_TX) = c;
	if(c == '\n') {
		WAIT_FOR_THRE;
		REG8(UART_BASEADDR + UART_TX) = '\r';
	}
	WAIT_FOR_XMITR;
	return 1;
}

char uart_getc(void)
{
  	unsigned char lsr;
  	char c;

  	WAIT_FOR_CHAR;
  	c = REG8(UART_BASEADDR + UART_RX);
  	  
  	return c;
}

void uart_puts(char *c)
{
	//while(*c != '\0')
	while(*c != NULL)
    {
        uart_putc(*c);
        ++c;
    }
}
