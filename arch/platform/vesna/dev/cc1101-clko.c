/*
 * Copyright (c) 2023, ComLab, Jozef Stefan Institute - https://e6.ijs.si/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *        Minimal drivers for CC1101 radio on VESNA SNE_ISMTV boards.
 *        The radio is used for CLK output - to feed it into VESNA SNC.
 *        The clock output on GDO0 will be set to 13.5 MHz. For other possible 
 *        freq see radio datasheet.
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "vsnspi_new.h"
#include "cc1101-clko.h"
/*---------------------------------------------------------------------------*/
#include "sys/log.h"
#define LOG_LEVEL   LOG_LEVEL_INFO
#define LOG_MODULE  "cc1101"
/*---------------------------------------------------------------------------*/
#define CC1101_SPI_SPEED    ((uint32_t)8000000)
/*---------------------------------------------------------------------------*/
// SPI struct (from VESNA drivers) for CC1101 radio
static vsnSPI_CommonStructure CC1101_SPI_Structure;
vsnSPI_CommonStructure * const CC1101_SPI = &CC1101_SPI_Structure;
/*---------------------------------------------------------------------------*/
static void
CC1101_spiErrorCallback(void *cbDevStruct)
{
	vsnSPI_CommonStructure *spi = cbDevStruct;
	vsnSPI_chipSelect(spi, SPI_CS_HIGH);
    LOG_ERR("CC1101\n");
}
/*---------------------------------------------------------------------------*/
static void
CC1101_clearCS(void)
{
    vsnSPI_chipSelect(CC1101_SPI, SPI_CS_HIGH);
}
/*---------------------------------------------------------------------------*/
static void
CC1101_setCS(void)
{
    vsnSPI_chipSelect(CC1101_SPI, SPI_CS_LOW);
}
/*---------------------------------------------------------------------------*/
static void
CC1101_regWrite(uint8_t addr, uint8_t value)
{
    vsnSPI_ErrorStatus CC_status;
    uint8_t dummy __attribute__((unused));
    uint8_t state;

    CC1101_setCS();

    int count = 0;			
    while((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)) && (count < 200000))count++;										
    if(count >= 200000) LOG_ERR("_regWrite: MISO is not low! \n");											

    CC_status = vsnSPI_pullByteTXRX(CC1101_SPI, addr , &state);
    if (VSN_SPI_SUCCESS != CC_status) LOG_WARN("ERR while sending\n");
    LOG_DBG("regWrite address state is 0x%02x  \n",state);

    CC_status = vsnSPI_pullByteTXRX(CC1101_SPI, value, &state);
    if (VSN_SPI_SUCCESS != CC_status) LOG_WARN("ERR while receiving\n");
    LOG_DBG("regWrite data state is 0x%02x  \n",state);

    CC1101_clearCS();        
}
/*---------------------------------------------------------------------------*/
static void
CC1101_reset(void)
{
    uint8_t dummy __attribute__((unused));

    // SCLK = 1 and MOSI = 0
	GPIO_SetBits(GPIOA, GPIO_Pin_5);
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);

    // Strobe CS low/high
	GPIO_SetBits(GPIOB, GPIO_Pin_9);
	vsnTime_delayUS(10);
	GPIO_ResetBits(GPIOB, GPIO_Pin_9);
	vsnTime_delayUS(10); 

    // Hold CS low for at least 40us
	GPIO_SetBits(GPIOB, GPIO_Pin_9);
	vsnTime_delayUS(100); 

    // Pull CS low and wait for MISO to go low
	GPIO_ResetBits(GPIOB, GPIO_Pin_9);
	int count = 0;
    while((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)) && (count < 200000)) count++;										
    if(count >= 200000) LOG_ERR("_reset: MISO is not low!\n");		

	vsnTime_delayUS(300);

    CC1101_setCS();
       
    count = 0;			
    while((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)) && (count < 200000)) count++;										
    if(count >= 200000) LOG_ERR("_reset: MISO is not low! \n");

    //Isue SRES (0x30) strobe on MOSI line
    vsnSPI_pullByteTXRX(CC1101_SPI, 0x30 , &dummy);

    vsnTime_delayUS(50);
    
    CC1101_clearCS();
	
    // When MISO goes low again, reset is complete
    count = 0;    
    while((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)) && (count < 500000)) count++;										
    if(count >= 500000) LOG_ERR("_reset: MISO is not low!\n");												
    
	vsnTime_delayUS(300);

    LOG_INFO("Radio reset complete!\n");
}
/*---------------------------------------------------------------------------*/


void
cc1101_init_clk_output(void)
{
	static SPI_InitTypeDef spiConfig = {
		//.SPI_BaudRatePrescaler is overwritten later
		.SPI_Direction = SPI_Direction_2Lines_FullDuplex,
		.SPI_Mode = SPI_Mode_Master,
		.SPI_DataSize = SPI_DataSize_8b,
		.SPI_CPOL = SPI_CPOL_Low,
		.SPI_CPHA = SPI_CPHA_1Edge,
		.SPI_NSS = SPI_NSS_Soft,
		.SPI_FirstBit = SPI_FirstBit_MSB,
		.SPI_CRCPolynomial = 7,
	};

    vsnSPI_initCommonStructure(
        CC1101_SPI,
        CC1101_SPI_PORT,
        &spiConfig,
        CC1101_CSN_PIN,
        CC1101_CSN_PORT,
        CC1101_SPI_SPEED     //speed can be the same as rf2xx
    );

    // Init SPI for CC radio
    vsnSPI_initHW(VSN_SPI1);
    vsnSPI_Init(CC1101_SPI, CC1101_spiErrorCallback);

    // Set GDO0 output pin to desire freq (0x32 = 13.5MHz)
    CC1101_setCS();

    // Radio must be in IDLE state
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == 0) {
        CC1101_regWrite(0x02, 0x32);
    }
    else {
        // First reset the radio and then set the freq
        CC1101_reset();
        CC1101_regWrite(0x02, 0x32);
    }
    CC1101_clearCS();

    LOG_INFO("CLK (GDO0) output freq set to 13.5 MHz\n");

    // Give SPI control back to rf2xx
    vsnSPI_deInit(CC1101_SPI);
}
