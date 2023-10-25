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
 *        This file provides connectivity information on LEDs, Buttons, UART,
 *        radios and other peripherals on VESNA SNE_ISMTV ver 1.0 boards.
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 *
 *
 * VESNA connector pin-out on ISMTV-V1.0 :
 *            +-----+------+
 *           || PB3 | PB4  || JTAG
 *   CC_CSN  || PB9 | PA14 || JTAG
 *           || PB8 | PA15 || JTAG & AT_SLPT
 *           || RST | PA13 || JTAG & AT_RSTN 
 *   MOSI/SI || PA7 | PA6  || MISO/SO
 *   SCLK    || PA5 | PA4  || CC_GDO2
 *   AT_IRQ  || PA3 | PA2  || CC_GDO0
 *           || PA1 | PA0  || AT_CS
 *           || GND | GND  || 
 *           || Vch | VPP  || 
 *           || V24 | VCC  || 
 *            +-----+------+
 */
/*---------------------------------------------------------------------------*/
#ifndef BOARD_H_
#define BOARD_H_
/*---------------------------------------------------------------------------*/
/* Board specific defines - pin mapping                                      */
/*---------------------------------------------------------------------------*/

/* Both TI and ATMEL radios are connected to SPI1 */
#define AT86RF2XX_SPI_PORT			(VSN_SPI1)
#define CC1101_SPI_PORT			    (VSN_SPI1)

/* ATMEL CSn - inverted chip select */
#define AT86RF2XX_CSN_PIN			(GPIO_Pin_0)
#define AT86RF2XX_CSN_PORT			(GPIOA)

/* ATMEL DIG2 - optional for timing */ 
#define AT86RF2XX_DIG2_PIN			(GPIO_Pin_1)
#define AT86RF2XX_DIG2_PORT			(GPIOA)

/* ATMEL RSTn - inverted RESET */
#define AT86RF2XX_RSTN_PIN			(GPIO_Pin_13)
#define AT86RF2XX_RSTN_PORT			(GPIOA)

/* ATMEL SLP_TR - Multi-functional pin */
#define AT86RF2XX_SLP_TR_PIN		(GPIO_Pin_15)
#define AT86RF2XX_SLP_TR_PORT		(GPIOA)

/* ATMEL IRQ - interrupts from radio */
#define AT86RF2XX_IRQ_PIN			(GPIO_Pin_3)
#define AT86RF2XX_IRQ_PORT			(GPIOA)

/* Mapping ATMEL IRQ to ISR */
#define AT86RF2XX_EXTI_IRQ_LINE    	(EXTI_Line3)
#define AT86RF2XX_EXTI_IRQ_PORT    	(GPIO_PortSourceGPIOA)
#define AT86RF2XX_EXTI_IRQ_PIN    	(GPIO_PinSource3)
#define AT86RF2XX_EXTI_IRQ_CHANNEL  (EXTI3_IRQn)


/* TI CSn - inverted chip select */
#define CC1101_CSN_PIN			    (GPIO_Pin_9)
#define CC1101_CSN_PORT			    (GPIOB)

/* TI GDO0 - can be used for clock output (CLKO) */
#define CC1101_GDO0_PIN			    (GPIO_Pin_2)
#define CC1101_GDO0_PORT			(GPIOA)

/* TI GDO2 */
#define CC1101_GDO2_PIN			    (GPIO_Pin_4)
#define CC1101_GDO2_PORT			(GPIOA)

/*---------------------------------------------------------------------------*/
/* VESNA configuration for specific board                                    */
/*---------------------------------------------------------------------------*/
/* Use faster UART baudrate */
#define VESNA_CONF_UART1_BAUDRATE           (460800)

/* Use external clock source for rtimer */
#define VESNA_CONF_RTIMER_USE_EXTERNAL_SOURCE   (1)

/*---------------------------------------------------------------------------*/
/* Configuration required by Contiki-NG                                      */
/*---------------------------------------------------------------------------*/
#include "at86rf2xx-def.h"

#define NETSTACK_CONF_RADIO 	            at86rf2xx_driver

#define RADIO_DELAY_BEFORE_RX               AT86RF2XX_DELAY_BEFORE_RX
#define RADIO_DELAY_BEFORE_TX               AT86RF2XX_DELAY_BEFORE_TX
#define RADIO_DELAY_BEFORE_DETECT           AT86RF2XX_DELAY_BEFORE_DETECT
#define RADIO_BYTE_AIR_TIME                 AT86RF2XX_BYTE_AIR_TIME
#define RADIO_PHY_OVERHEAD                  AT86RF2XX_PHY_OVERHEAD

#endif /* BOARD_H_ */
