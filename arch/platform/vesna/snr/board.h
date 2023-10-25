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
 *        radios and other peripherals on VESNA SNR boards.
 * \author
 *        Grega Morano <grega.morano@ijs.si>

 * Side VESNA connector pin-out on SNR :
 *  +-------+------+
 *  | PB7   | PC11 |
 *  | PB6   | PC10 |
 *  | VCC   | GND  |
 *  | PPB15 | PB12 |
 *  | PB13  | PC9  |
 *  | PB14  | PD0  |
 *  +-------+------+
 */
/*---------------------------------------------------------------------------*/
#ifndef BOARD_H_
#define BOARD_H_

/*---------------------------------------------------------------------------*/
/* Board specific defines for the radio on the platform - pin mapping*/
#define AT86RF2XX_SPI_PORT 			(VSN_SPI2)

// Inverted chip select
#define AT86RF2XX_CSN_PIN			(GPIO_Pin_12)
#define AT86RF2XX_CSN_PORT			(GPIOB)

// (Optional) DIG2 -- for timing
#define AT86RF2XX_DIG2_PIN			(GPIO_Pin_7)
#define AT86RF2XX_DIG2_PORT			(GPIOB)

// RSTN -- inverted RESET
#define AT86RF2XX_RSTN_PIN			(GPIO_Pin_11)
#define AT86RF2XX_RSTN_PORT			(GPIOC)

// Multi-functional pin
#define AT86RF2XX_SLP_TR_PIN		(GPIO_Pin_10)
#define AT86RF2XX_SLP_TR_PORT	    (GPIOC)

// IRQ -- interrupts from radio
#define AT86RF2XX_IRQ_PIN			(GPIO_Pin_9)
#define AT86RF2XX_IRQ_PORT			(GPIOC)

// Mapping IRQ to ISR
#define AT86RF2XX_EXTI_IRQ_LINE		(EXTI_Line9)
#define AT86RF2XX_EXTI_IRQ_PORT		(GPIO_PortSourceGPIOC)
#define AT86RF2XX_EXTI_IRQ_PIN		(GPIO_PinSource9)
#define AT86RF2XX_EXTI_IRQ_CHANNEL	(EXTI9_5_IRQn)

// CLKM - clock output (defined but not used anywhere)
#define AT86RF2XX_CLKM_PIN			(GPIO_Pin_0)
#define AT86RF2XX_CLKM_PORT			(GPIOD)

/*---------------------------------------------------------------------------*/

/* VESNA SNC will use external clock source - provided by the radio  */
#define VESNA_CONF_USE_EXTERNAL_CLOCK       (1)

/* Since SNC uses precise clock, no external source for rtimer is needed */
#define VESNA_CONF_RTIMER_USE_EXTERNAL_SOURCE   (0)

/* Use faster UART baudrate */
#define VESNA_CONF_UART1_BAUDRATE           (460800)

/*---------------------------------------------------------------------------*/
/* Radio configuration required by Contiki-NG */
#include "at86rf2xx-def.h"

#define NETSTACK_CONF_RADIO 	            at86rf2xx_driver

#define RADIO_DELAY_BEFORE_RX               AT86RF2XX_DELAY_BEFORE_RX
#define RADIO_DELAY_BEFORE_TX               AT86RF2XX_DELAY_BEFORE_TX
#define RADIO_DELAY_BEFORE_DETECT           AT86RF2XX_DELAY_BEFORE_DETECT
#define RADIO_BYTE_AIR_TIME                 AT86RF2XX_BYTE_AIR_TIME
#define RADIO_PHY_OVERHEAD                  AT86RF2XX_PHY_OVERHEAD

/* Configure radio to enable CLKM output -- needed by VESNA_USE_EXTERNAL_CLOCK*/
#define AT86RF2XX_CONF_ENABLE_CLKM          (1)


#endif /* BOARD_H_ */
