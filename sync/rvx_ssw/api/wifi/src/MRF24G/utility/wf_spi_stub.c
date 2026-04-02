/*******************************************************************************
 MRF24WG SPI Stub Functions

  Summary: Functions to control MRF24WG RESET and HIBERNATE pins need by the
           Universal Driver.  Functions in this module should not be called
           directly by the application code.

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
#include "System.h"

/*****************************************************************************
  Function:
    void WF_SpiInit(void);

  Summary:
    Initializes SPI controller used to communicate with MRF24WG

  Description:
    Called by Universal Driver during initialization to initialize SPI interface.

 Parameters:
    None

  Returns:
    None
    Should not be called directly from application code.
*****************************************************************************/
#include "platform_info.h"
#include "frvp_spi.h"
#include "ervp_mmio_util.h"

/* The SPI SCK speed can be up to 25 MHz. */
#define SPI_FREQ_OF_WIFI		(6250000)
#define SPI_MODE_OF_WIFI		(SPI_SCKMODE_0)
#define SPI_PORT_ID_OF_WIFI		(SPI_INDEX_FOR_WIFI)

static SpiConfig wifi_spi_config = {SPI_DIVSOR(SPI_FREQ_OF_WIFI), SPI_MODE_OF_WIFI, (1<<SPI_PORT_ID_OF_WIFI), SPI_CSMODE_OFF, (SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB) | SPI_FMT_LEN(8)), 1};

void WF_SpiInit(void)
{
    spi_configure(&wifi_spi_config);
}

/*****************************************************************************
  Function:
    void WF_SpiEnableChipSelect(void);

  Summary:
    Selects the MRF24WG SPI by setting the CS line low.

  Description:
    Called by Universal Driver when preparing to transmit data to the MRF24WG.

 Parameters:
    None

  Returns:
    None

  Remarks:
    Should not be called directly from application code.
*****************************************************************************/
void WF_SpiEnableChipSelect(void)
{
	spi_start();
}

/*****************************************************************************
  Function:
    void WF_SpiDisableChipSelect(void);

  Summary:
    Deselects the MRF24WG SPI by setting CS high.

  Description:
    Called by Universal Driver after completing an SPI transaction.
e
 Parameters:
    None

  Returns:
    None

  Remarks:
    Should not be called directly from application code.
*****************************************************************************/
void WF_SpiDisableChipSelect(void)
{
	spi_end();
}

/*****************************************************************************
  Function:
    void WF_SpiTxRx(uint8_t *p_txBuf, uint16_t txLength, uint8_t *p_rxBuf, uint16_t rxLength);

  Summary:
    Transmits and receives SPI bytes with the MRF24WG.

  Description:
    Called by Universal Driver to communicate with the MRF24WG.

 Parameters:
    p_txBuf  -- pointer to the transmit buffer
    txLength -- number of bytes to be transmitted from p_txBuf
    p_rxBuf  -- pointer to receive buffer
    rxLength -- number of bytes to read and copy into p_rxBuf

  Returns:
    None

  Remarks:
    Should not be called directly from application code.
*****************************************************************************/

void WF_SpiTxRx(const uint8_t *p_txBuf, uint16_t txLength, uint8_t *p_rxBuf,uint16_t rxLength){
	uint16_t i;
    uint16_t byteCount;
    byteCount = (txLength >= rxLength)?txLength:rxLength;

    spi_read_and_write(byteCount, p_txBuf, p_rxBuf);
}
