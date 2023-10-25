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
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 *
 *        The uC clock of VESNA SNC drifts quite a lot and therefore the TSCH
 *        communication becomes unreliable. The SNE_ISMTV boards are equipped
 *        with both AT86RF2xx and CC1101 radios, however, only the CC1101 has 
 *        CLK output pin connected to VESNA uC. By using CC1101 and its CLK 
 *        output we can drive the Contiki's rtimer with a stable clock source.
 */
/*---------------------------------------------------------------------------*/
#ifndef CC1101_HAL_H_
#define CC1101_HAL_H_

/**
 * \brief Configure the CLK output of CC1101 radio on SNE_ISMTV_1.x
 *
 * The function will setup all the required peripherals (GPIOs, SPI) for CC 
 * radio. Then it will configure the radio itself to output desired clock
 * output. At the end, it will clean unused resources.
 * Before calling this function, most of the uC peripherals (UART!) should 
 * already be configured. 
 * 
 * The function can be used only on ISMTV boards and should be called only 
 * once - at setup, (just) before the HW initialization of AT86RF2xx radios.
 */
void cc1101_init_clk_output(void);

#endif /* CC1101_HAL_H_ */