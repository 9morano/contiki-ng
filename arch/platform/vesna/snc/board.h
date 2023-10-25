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
 *        radios and other peripherals on standalone VESNA boards.
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 *
 *
 *  VESNA connector pin-out:
 *  +-----+------+
 *  | PB3 | PB4  |
 *  | PB9 | PA14 |
 *  | PB8 | PA15 |
 *  | RST | PA13 |
 *  | PA7 | PA6  |
 *  | PA5 | PA4  |
 *  | PA3 | PA2  |
 *  | PA1 | PA0  |
 *  | GND | GND  |
 *  | Vch | VPP  |
 *  | V24 | VCC  |
 *  +-----+------+
 *
 *  Side VESNA connector pin-out:
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

#define NETSTACK_CONF_RADIO        nullradio_driver

#endif /* BOARD_H_ */
