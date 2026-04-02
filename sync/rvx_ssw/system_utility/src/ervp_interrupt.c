#include <stddef.h>
#include "ervp_interrupt.h"

#include "ervp_printf.h"
#include "ervp_core_id.h"
#include "ervp_multicore_synch.h"
#include "ervp_external_peri_group_memorymap.h"
#include "frvp_plic_memorymap_offset.h"
#include "ervp_assert.h"

extern void trap_entry();

#ifdef INCLUDE_TIMER
#include "ervp_timer.h"
static void handle_timer_interrupt();
#endif

#ifdef INCLUDE_PLIC
#include "frvp_plic.h"
static function_ptr_t external_interrupt_isr[PLIC_NUM_INTERRUPTS];
static plic_instance_t plic_config;
static void no_interrupt_handler (void) {};

static void handle_external_interrupt();

#define GPIO_ISR_BASE (PLIC_INDEX_GPIO+1)
#define USER_ISR_BASE (PLIC_INDEX_USER+1)
#define WIFI_ISR_BASE (PLIC_INDEX_WIFI+1)
#define HBC1_ISR_BASE (PLIC_INDEX_HBC1+1)
#define I2S_ISR_BASE (PLIC_INDEX_I2S+1)

#endif

#ifdef INCLUDE_GPIO
#include "ervp_user_gpio.h"
#endif

#ifdef INCLUDE_WIFI
#include "ervp_wifi.h"
#endif

//////////
/* body */
//////////

__attribute__((weak)) void wait_for_interrupts()
{
}

#ifdef INCLUDE_TIMER
static void handle_timer_interrupt()
{
	function_ptr_t isr;
	/* Disable the machine and timer interrupts until setup is completed */
	disallow_interrupt_timer();
	clear_timer_interrupt();
	/* Notify the main loop that it can now  printf*/
	isr = (function_ptr_t)REG32(MMAP_TIMER_ISR);
	if(isr!=0)
		isr();
	/* Re-enable timer */
	allow_interrupt_timer();
}

void register_isr_timer(function_ptr_t isr)
{
	REG32(MMAP_TIMER_ISR) = (unsigned int)isr;
}

#endif

#ifdef INCLUDE_PLIC

static inline unsigned int get_gpio_itr_index(int gpio_index)
{
	return (GPIO_ISR_BASE + gpio_index);
}

static inline unsigned int get_gpio_index(int itr_index)
{
	return (itr_index - GPIO_ISR_BASE);
}

static inline unsigned int get_user_itr_index(int user_index)
{
	return (USER_ISR_BASE + user_index);
}

static void handle_external_interrupt(){
	function_ptr_t isr;
  plic_source int_num  = PLIC_claim_interrupt(&plic_config);
  if (int_num < PLIC_NUM_INTERRUPTS)
	{
		isr = external_interrupt_isr[int_num];
		if(isr!=0)
			isr();
#ifdef INCLUDE_GPIO
		if((int_num>=GPIO_ISR_BASE) && (int_num<USER_ISR_BASE))
			user_gpio_clear_interrupt(get_gpio_index(int_num));
#endif
#ifdef INCLUDE_WIFI
		if(int_num==WIFI_ISR_BASE)
			wifi_clear_interrupt();
#endif
	}
  else {
    exit(1 + (uintptr_t) int_num);
  }
  PLIC_complete_interrupt(&plic_config, int_num);
}

void register_isr_gpio(int gpio_index, function_ptr_t isr, int priority)
{
	int itr_index = get_gpio_itr_index(gpio_index);
	external_interrupt_isr[itr_index] = isr;
	enable_isr_gpio(gpio_index);
	PLIC_set_priority(&plic_config, itr_index, priority);
}

void enable_isr_gpio(int gpio_index)
{
	int itr_index = get_gpio_itr_index(gpio_index);
	PLIC_enable_interrupt(&plic_config, itr_index);
}

void disable_isr_gpio(int gpio_index)
{
	int itr_index = get_gpio_itr_index(gpio_index);
	PLIC_disable_interrupt(&plic_config, itr_index);
}

void register_isr_user(int user_index, function_ptr_t isr, int priority)
{
	int itr_index = get_user_itr_index(user_index);
	external_interrupt_isr[itr_index] = isr;
	enable_isr_user(user_index);
	PLIC_set_priority(&plic_config, itr_index, priority);
}

void enable_isr_user(int user_index)
{
	int itr_index = get_user_itr_index(user_index);
	PLIC_enable_interrupt(&plic_config, itr_index);
}

void disable_isr_user(int user_index)
{
	int itr_index = get_user_itr_index(user_index);
	PLIC_disable_interrupt(&plic_config, itr_index);
}

void register_isr_wifi(function_ptr_t isr, int priority)
{
	int itr_index = WIFI_ISR_BASE;
	external_interrupt_isr[itr_index] = isr;
	PLIC_enable_interrupt(&plic_config, itr_index);
	PLIC_set_priority(&plic_config, itr_index, priority);
}

void enable_isr_wifi()
{
	int itr_index = WIFI_ISR_BASE;
	PLIC_enable_interrupt(&plic_config, itr_index);
}

void disable_isr_wifi()
{
	int itr_index = WIFI_ISR_BASE;
	PLIC_disable_interrupt(&plic_config, itr_index);
}

void register_isr_hbc1(function_ptr_t isr, int priority)
{
	int itr_index = HBC1_ISR_BASE;
	external_interrupt_isr[itr_index] = isr;
	PLIC_enable_interrupt(&plic_config, itr_index);
	PLIC_set_priority(&plic_config, itr_index, priority);
}

void enable_isr_hbc1()
{
	int itr_index = HBC1_ISR_BASE;
	PLIC_enable_interrupt(&plic_config, itr_index);
}

void disable_isr_hbc1()
{
	int itr_index = HBC1_ISR_BASE;
	PLIC_disable_interrupt(&plic_config, itr_index);
}

void register_isr_i2s(function_ptr_t isr, int priority)
{
	int itr_index = I2S_ISR_BASE;
	external_interrupt_isr[itr_index] = isr;
	PLIC_enable_interrupt(&plic_config, itr_index);
	PLIC_set_priority(&plic_config, itr_index, priority);
}

void enable_isr_i2s()
{
	int itr_index = I2S_ISR_BASE;
	PLIC_enable_interrupt(&plic_config, itr_index);
}

void disable_isr_i2s()
{
	int itr_index = I2S_ISR_BASE;
	PLIC_disable_interrupt(&plic_config, itr_index);
}

#endif

void init_handle_trap()
{
	write_csr(mtvec, &trap_entry);
	if(EXCLUSIVE_ID==0)
	{
#ifdef INCLUDE_PLIC
		for (int ii = 0; ii < PLIC_NUM_INTERRUPTS; ii ++){
			external_interrupt_isr[ii] = no_interrupt_handler;
		}
		PLIC_init(&plic_config,
				PLIC_BASEADDR,
				PLIC_NUM_INTERRUPTS,
				PLIC_NUM_PRIORITIES);
#endif
	}
}

uintptr_t handle_trap(uintptr_t mcause, uintptr_t mip, uintptr_t epc)
{
  if (0){
#ifdef INCLUDE_PLIC
    // External Machine-Level interrupt from PLIC
  } else if ((mcause & MCAUSE_INT) && ((mip & MIP_MEIP) > 0)) {
    handle_external_interrupt();
#endif
#ifdef INCLUDE_TIMER
		// External Machine-Level interrupt from PLIC
  } else if ((mcause & MCAUSE_INT) && ((mip & MIP_MTIP) > 0)){
    handle_timer_interrupt();
#endif
  }
	else {
		printf_must("\nUnhandled Trap: %x", mcause);
		printf_must("\nmip: %08x", mip);
		printf_must("\nepc: %08x", epc);
		while(1);
	}
	return epc;
}

#ifdef INCLUDE_MULTICORE

static int plic_core_id = -1;

void register_plic_grant()
{
	acquire_lock(LOCK_INDEX_FOR_SYSTEM_VARIABLE);
	assert(plic_core_id==(-1)); // Only one core can get the grant!
	plic_core_id = EXCLUSIVE_ID;
	release_lock(LOCK_INDEX_FOR_SYSTEM_VARIABLE);
}

void check_plic_grant()
{
	assert(EXCLUSIVE_ID==plic_core_id); // Wrong PLIC grant!
}

#endif
