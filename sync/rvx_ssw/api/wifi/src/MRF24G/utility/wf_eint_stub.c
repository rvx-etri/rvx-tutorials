/*******************************************************************************
 MRF24WG External Interrupt Stub Functions

  Summary: Functions to control MRF24WG External Interrupt.

 *******************************************************************************/

/* MRF24WG0M Universal Driver
*
* Copyright (c) 2012-2013, Microchip <www.microchip.com>
* Contact Microchip for the latest version.
*
* This program is free software; distributed under the terms of BSD
* license:
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1.    Redistributions of source code must retain the above copyright notice, this
*        list of conditions and the following disclaimer.
* 2.    Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
* 3.    Neither the name(s) of the above-listed copyright holder(s) nor the names
*        of its contributors may be used to endorse or promote products derived
*        from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//==============================================================================
//                                  INCLUDES
//==============================================================================
// board-specific includes

#include "ervp_interrupt.h"
#include "core_dependent.h"
#include "ervp_external_peri_group_memorymap.h"
#include "frvp_spi.h"
#include "ervp_wifi.h"

#include "System.h"

void WF_Interrupt(void){
	wifi_set_interrupt_pending(1);
	SpiConfig backup_config;
	spi_store_config(&backup_config);
	WF_SpiInit();
	WF_EintHandler();           // call Univeral Driver handler function
	spi_restore_config(&backup_config);
}

/*****************************************************************************
  Function:
    void WF_EintInit(void);

  Summary:
    Configure host processor external interrupt.  This line is asserted low by
    MRF24WG.

  Description:
    Called by Universal Driver during initialization.

 Parameters:
    None

  Returns:
    None

  Remarks:
    Should not be called directly from application code.
*****************************************************************************/
void WF_EintInit(void)
{
	int itr_priority = 1;
	disable_interrupt();

	register_isr_wifi((function_ptr_t)WF_Interrupt, itr_priority);
	wifi_set_interrupt_pending(0);
	allow_interrupt_plic();

	enable_interrupt();
	return;
}



/*****************************************************************************
  Function:
    void WF_EintEnable(void);

  Summary:
    Enables the MRF24WG external interrupt

  Description:
    Called by Universal Driver during normal operations to enable the MRF24WG
    external interrupt.  
 
    Note: When using level-triggered interrupts it is possible that host MCU
          could miss a falling edge; this can occur because during normal
          operations as the Universal Driver disables the external interrupt for
          short periods.  If the MRF24WG asserts the interrupt line while the
          interrupt is disabled the falling edge won't be detected.  So, a
          check must be made to determine if an interrupt is pending; if so, the
          interrupt must be forced.

          This is not an issue for level-triggered interrupts.

 Parameters:
    None

  Returns:
    None

  Remarks:
    Should not be called directly from application code.
*****************************************************************************/
void WF_EintEnable(void)
{
	wifi_set_interrupt_pending(0);
}

/*****************************************************************************
  Function:
    void WF_EintDisable(void);

  Summary:
    Disables the MRF24WG external interrupt

  Description:
    Called by Universal Driver during normal operations to disable the MRF24WG
    external interrupt.

 Parameters:
    None

  Returns:
    None

  Remarks:
    Should not be called directly from application code.
*****************************************************************************/
void WF_EintDisable(void)
{
	wifi_set_interrupt_pending(1);
}

/*****************************************************************************
  Function:
    bool WF_isEintDisabled(void);

  Summary:
    Determines if the external interrupt is disabled

  Description:
    Called by Universal Driver during normal operations to check if the current
    state of the external interrupt is disabled.

 Parameters:
    None

  Returns:
    True if interrupt is disabled, else False

  Remarks:
    Should not be called directly from application code.
*****************************************************************************/

bool WF_isEintDisabled(void)
{
	return wifi_is_interrupt_pending();
}

/*****************************************************************************
  Function:
    bool WF_isEintPending(void);

  Summary:
    Determines if the external interrupt is pending.

  Description:
    Called by Universal Driver during normal operations to check if an external
    interrupt is pending.

 Parameters:
    None

  Returns:
    True if interrupt is pending, else False

  Remarks:
    Should not be called directly from application code.
*****************************************************************************/
#if 0
bool WF_isEintPending(void)
{
    // if interrupt line is low, but interrupt request has not occurred
//    return(((WF_INT_IO == 0) && (WIFI_INT_IF == 0)));
	return 0; // unuse
}
#endif
