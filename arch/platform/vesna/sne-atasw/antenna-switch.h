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
 *        PE426482 antenna switch driver.        
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 *
 *        PE426482 is absorptive SP8T RF switch with:
 *        - freq from 10 MHz to 8 GHz 
 *        - fast switching times: 100 ns
 *        - insertion losses 1.1 @ 2.4 GHz
 *        - isolation: 38 db @ 2.4 GHz
 *        - requires 120 uA @ 3.3V ~ 5V 
 *
 *        On SNE_ATASW board, the RF switch must be powered on via GPIO pin.
 */
/*---------------------------------------------------------------------------*/
#ifndef ANTENNA_SWITCH_H_
#define ANTENNA_SWITCH_H_
/**
 * \brief Initialise the GPIOs for antenna switch. Enable power supply. 
 */
void asw_init(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Select the desired antenna element. Defaults to A1 (number 0)
 * \param element antenna numbers map from A1-A8 to 0-7
 */
void asw_select(uint8_t element);

#endif /* ANTENNA_SWITCH_H_ */
