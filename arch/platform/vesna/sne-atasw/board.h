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
 * VESNA connector pin-out on ATASW :
 *            +-----+------+
 *   AT_CLKO || PB3 | PB4  || JTAG
 *   SW_V2   || PB9 | PA14 || JTAG
 *   SW_V3   || PB8 | PA15 || JTAG
 *           || RST | PA13 || JTAG
 *   AT_MOSI || PA7 | PA6  || AT_MISO
 *   AT_SCLK || PA5 | PA4  || AT_RSTn
 *   AT_IRQ  || PA3 | PA2  || SW_VCC
 *   SW_V1   || PA1 | PA0  || AT_SELn
 *           || GND | GND  || 
 *           || Vch | VPP  || 
 *           || V24 | VCC  || 
 *            +-----+------+
 *
 */
/*---------------------------------------------------------------------------*/
#ifndef BOARD_H_
#define BOARD_H_
/*---------------------------------------------------------------------------*/
/* Board specific defines - pin mapping                                      */
/*---------------------------------------------------------------------------*/

/* ATMEL radio is connected to SPI1 */
#define AT86RF215_SPI_PORT			(VSN_SPI1)

/* CSn - inverted chip select */
#define AT86RF215_CSN_PIN			(GPIO_Pin_0)
#define AT86RF215_CSN_PORT			(GPIOA)

/* RSTn - inverted RESET */
#define AT86RF215_RSTN_PIN			(GPIO_Pin_4)
#define AT86RF215_RSTN_PORT			(GPIOA)

/* IRQ - interrupts from radio */
#define AT86RF215_IRQ_PIN			(GPIO_Pin_3)
#define AT86RF215_IRQ_PORT			(GPIOA)

/* Mapping IRQ to ISR */
#define AT86RF215_EXTI_IRQ_LINE    	(EXTI_Line3)
#define AT86RF215_EXTI_IRQ_PORT    	(GPIO_PortSourceGPIOA)
#define AT86RF215_EXTI_IRQ_PIN    	(GPIO_PinSource3)
#define AT86RF215_EXTI_IRQ_CHANNEL  (EXTI3_IRQn)

/* RF Switch GPIOs */
#define ATASW_SW_V1_PIN             (GPIO_Pin_1)
#define ATASW_SW_V1_PORT            (GPIOA)

#define ATASW_SW_V2_PIN             (GPIO_Pin_9)
#define ATASW_SW_V2_PORT            (GPIOB)

#define ATASW_SW_V3_PIN             (GPIO_Pin_8)
#define ATASW_SW_V3_PORT            (GPIOB)

/* RF Switch must be powered via GPIO pin */
#define ATASW_SW_VCC_PIN            (GPIO_Pin_2)
#define ATASW_SW_VCC_PORT           (GPIOA)



/*---------------------------------------------------------------------------*/
/* VESNA configuration for specific board                                    */
/*---------------------------------------------------------------------------*/
/* Use faster UART baudrate */
#define VESNA_CONF_UART1_BAUDRATE           (460800)

/* Radio can provide external clock source for VESNA SNC*/
#define VESNA_CONF_USE_EXTERNAL_CLOCK       (1)



/*---------------------------------------------------------------------------*/
/* Configuration required by Contiki-NG                                      */
/*---------------------------------------------------------------------------*/

#endif /* BOARD_H_ */
