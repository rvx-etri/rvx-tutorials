#include "touch.h"
#include "frvp_plic.h"
#include "core_dependent.h"

#define NUM_TOUCH_IRQ           (5)

extern function_ptr_t g_ext_interrupt_handlers[PLIC_NUM_INTERRUPTS];
extern plic_instance_t g_plic;

void touch_init(function_ptr_t *touch_callback_func)
{
	g_ext_interrupt_handlers[NUM_TOUCH_IRQ] = touch_callback_func;
	PLIC_enable_interrupt(&g_plic, NUM_TOUCH_IRQ);
	PLIC_set_priority(&g_plic, NUM_TOUCH_IRQ, 1);
	allow_interrupt_plic();
	enable_interrupt();
}

void read_touch(unsigned int *dx, unsigned int *dy)
{
	*dx = *(volatile unsigned int *)(0x99000000);
	*dy = *(volatile unsigned int *)(0x99000004);
}
