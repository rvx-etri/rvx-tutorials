#include "platform_info.h"
#include "ervp_external_peri_group_memorymap_offset.h"

#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "ervp_uart_define.h"

#define NULL 0

#ifndef UART_BAUD_RATE
#define UART_BAUD_RATE (115200)
#endif
#ifndef UART_BASEADDR
#define UART_BASEADDR (EXTERNAL_PERI_GROUP_BASEADDR + SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_UART0)
#endif
/* UART 16550 driver */

#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

static inline unsigned int get_base_addr(int uart_index)
{
	const unsigned int addr_diff = SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_UART1 - SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_UART0;
	return UART_BASEADDR + uart_index*addr_diff;
}

int uart_check_rx_data_ready(int uart_index)
{
	unsigned char lsr;
	lsr = REG8(get_base_addr(uart_index) + UART_LSR);
	return (lsr & UART_LSR_DR) == UART_LSR_DR;
}

char uart_read_rx_buffer(int uart_index)
{
        char c;
        const unsigned int uart_addr = get_base_addr(uart_index);
        c = REG8(uart_addr + UART_RX);
        return c;
}

static inline void wait_for_xmitr(int uart_index)
{
	unsigned char lsr;
	do {
		lsr = REG8(get_base_addr(uart_index) + UART_LSR);
	} while ((lsr & BOTH_EMPTY) != BOTH_EMPTY);
	return;
}

static inline void wait_for_thre(int uart_index)
{
	unsigned char lsr;
	do {
		lsr = REG8(get_base_addr(uart_index) + UART_LSR);
	} while ((lsr & UART_LSR_THRE) != UART_LSR_THRE);
	return;
}

static inline void wait_for_char(int uart_index)
{
	unsigned char lsr;
	do {
		lsr = REG8(get_base_addr(uart_index) + UART_LSR);
	} while ((lsr & UART_LSR_DR) != UART_LSR_DR);
	return;
}

void uart_init(void)
{
	uart_config(UART_INDEX_FOR_UART_PRINTF, UART_BAUD_RATE);
}

void uart_config(int uart_index, unsigned int baud_rate)
{
	unsigned int divisor;
	unsigned int uart_addr;
	uart_addr = get_base_addr(uart_index);
	/* Reset receiver and transmiter */
#if 1
	REG8(uart_addr + UART_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14;	// UART 16550
#else
	//REG8(uart_addr + UART_FCR) = 0x00;	// UART 8250
#endif

	/* Disable all interrupts */
	REG8(uart_addr + UART_IER) = 0x00;

	/* Set 8 bit char, 1 stop bit, no parity */
	REG8(uart_addr + UART_LCR) = UART_LCR_WLEN8 & ~(UART_LCR_STOP | UART_LCR_PARITY);

	/* Set baud rate */
	divisor = (unsigned int)((float)UART_CLK_HZ/(16 * baud_rate)+0.5);
	//divisor = (unsigned int)((float)UART_CLK_HZ/(16 * baud_rate));
	REG8(uart_addr + UART_LCR) |= UART_LCR_DLAB;
	REG8(uart_addr + UART_DLL) = divisor & 0x000000ff;
	REG8(uart_addr + UART_DLM) = (divisor >> 8) & 0x000000ff;
	REG8(uart_addr + UART_LCR) &= ~(UART_LCR_DLAB);

	/* enable */
	REG8(uart_addr + UART_EN) = 1;
	//printf("%d\n", divisor);
}

int uart_putc(int uart_index, char c)
{
	const unsigned int uart_addr = get_base_addr(uart_index);
	wait_for_thre(uart_index);
	REG8(uart_addr + UART_TX) = c;
	wait_for_xmitr(uart_index);
	return 1;
}

char uart_getc(int uart_index)
{
	char c;
	const unsigned int uart_addr = get_base_addr(uart_index);
	wait_for_char(uart_index);
	c = REG8(uart_addr + UART_RX);
	return c;
}

void uart_puts(int uart_index, char *c)
{
	while(*c != NULL)
	{
		uart_putc(uart_index, *c);
		++c;
	}
}
