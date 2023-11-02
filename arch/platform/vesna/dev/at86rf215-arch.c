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
 *     AT86RF215 arch specific code for VESNA platform
 * 
 * \author
 *      Grega Morano <grega.morano@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "vsnspi_new.h"
/*---------------------------------------------------------------------------*/
#define AT86RF215_SPI_SPEED		((uint32_t)18000000) // Max supported is 25MHz      //TODOG
/*---------------------------------------------------------------------------*/

/* EXTI (interrupt) struct (from STM) and constant (immutable) pointer to it.
 * Used by multiple functions in this file */
static EXTI_InitTypeDef at86rf215_EXTI_Structure = {
    .EXTI_Line = AT86RF215_EXTI_IRQ_LINE,
    .EXTI_Mode = EXTI_Mode_Interrupt,
    .EXTI_Trigger = EXTI_Trigger_Rising,
    .EXTI_LineCmd = ENABLE,
};
EXTI_InitTypeDef * const at86rf215_EXTI = &at86rf215_EXTI_Structure;

/*---------------------------------------------------------------------------*/
/* SPI struct (from VESNA drivers) and constant (immutable) pointer to it.
 * Used by multiple functions in this file */
static vsnSPI_CommonStructure at86rf215_SPI_Structure;
vsnSPI_CommonStructure * const at86rf215_SPI = &at86rf215_SPI_Structure;

/*---------------------------------------------------------------------------*/
/* These settings are passed as pointer and they have to exist for the runtime
 * and does not change. That is why struct is `static`. */
static SPI_InitTypeDef at86rf215SpiConfig = {
    .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
    .SPI_Mode = SPI_Mode_Master,
    .SPI_DataSize = SPI_DataSize_8b,
    .SPI_CPOL = SPI_CPOL_Low,
    .SPI_CPHA = SPI_CPHA_1Edge,
    .SPI_NSS = SPI_NSS_Soft,
    .SPI_FirstBit = SPI_FirstBit_MSB,
    .SPI_CRCPolynomial = 7,
};
/*---------------------------------------------------------------------------*/
/* Error Call Back function - never called in this implementation, because we
 * are using vsnSPI_pullByteTXRX()... however required by SPI_init */
static void
spiErrorCallback(void *cbDevStruct)
{
	vsnSPI_CommonStructure *spi = cbDevStruct;
	vsnSPI_chipSelect(spi, SPI_CS_HIGH);
}
/*---------------------------------------------------------------------------*/
void 
at86rf215_arch_init(void)
{
    /* Enable GPIO pins (macros defined in board.h) */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

	/* RSTn (output) */
	GPIO_InitStructure.GPIO_Pin = AT86RF215_RSTN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(AT86RF215_RSTN_PORT, &GPIO_InitStructure);

	/* IRQ (input) */
	GPIO_InitStructure.GPIO_Pin = AT86RF215_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(AT86RF215_IRQ_PORT, &GPIO_InitStructure);

    // Important. I forgot this line and ISR was triggered by UART1 output signal.
	GPIO_EXTILineConfig(AT86RF215_EXTI_IRQ_PORT, AT86RF215_EXTI_IRQ_PIN);

	// Enable AT86RF2xx radio IRQ in NIVC
    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = AT86RF215_EXTI_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    /* Complete initialization of SPI for at86rf215 */ 
	vsnSPI_initHW(AT86RF215_SPI_PORT);
    vsnSPI_initCommonStructure(
        at86rf215_SPI,
        AT86RF215_SPI_PORT,
        &at86rf215SpiConfig,
        AT86RF215_CSN_PIN,
        AT86RF215_CSN_PORT,
        AT86RF215_SPI_SPEED
    );
	vsnSPI_Init(at86rf215_SPI, spiErrorCallback);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_enable_EXTI(void)
{
    at86rf215_EXTI->EXTI_LineCmd = ENABLE;
    EXTI_Init(at86rf215_EXTI);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_disable_EXTI(void)
{
    at86rf215_EXTI->EXTI_LineCmd = DISABLE;
    EXTI_Init(at86rf215_EXTI);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_clear_EXTI(void) //TODOG: ta je drugacna kot pri openmote
{
    EXTI_ClearFlag(AT86RF215_EXTI_IRQ_LINE);
}
/*---------------------------------------------------------------------------*/
void 
at86rf215_arch_set_RSTN(void)
{
    GPIO_ResetBits(AT86RF215_RSTN_PORT, AT86RF215_RSTN_PIN);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_clear_RSTN(void)
{
    GPIO_SetBits(AT86RF215_RSTN_PORT, AT86RF215_RSTN_PIN);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_spi_deselect(void)
{
    vsnSPI_chipSelect(at86rf215_SPI, SPI_CS_HIGH);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_spi_select(void)
{
    vsnSPI_chipSelect(at86rf215_SPI, SPI_CS_LOW);
}
/*---------------------------------------------------------------------------*/
uint8_t 
at86rf215_arch_spi_txrx(uint8_t tx)
{   
    uint8_t rx = 0;
    vsnSPI_pullByteTXRX(at86rf215_SPI, tx, &rx);
    return rx;
}
