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
 *        PE426482 antenna switch driver (for VESNA platform, SNE_ATASW board).
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 *
 *
 *        Control logic truth table for PE426482
 *             +----+----+----+
 *         ____|_V1_|_V2_|_V3_|          asw_select(x)
 *        | A1 |  0 |  0 |  0 |    -->         0
 *        | A2 |  0 |  0 |  1 |    -->         1
 *        | A3 |  0 |  1 |  0 |    -->         2
 *        | A4 |  0 |  1 |  1 |    -->         3
 *        | A5 |  1 |  0 |  0 |    -->         4
 *        | A6 |  1 |  0 |  1 |    -->         5
 *        | A7 |  1 |  1 |  0 |    -->         6
 *        | A8 |  1 |  1 |  1 |    -->         7
 *        +----+----+----+----+
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "stm32f10x_gpio.h"
#include "antenna-switch.h"
/*---------------------------------------------------------------------------*/
void asw_init(void)
{
    /* Enable GPIO pins (macros defined in board.h) */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_InitStructure.GPIO_Pin = ASW_V1_PIN;
	GPIO_Init(ASW_V1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ASW_V2_PIN;
	GPIO_Init(ASW_V2_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ASW_V3_PIN;
	GPIO_Init(ASW_V3_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ASW_VCC_PIN;
	GPIO_Init(ASW_VCC_PORT, &GPIO_InitStructure);

    /* Power-up the RF switch - put VCC pin to high*/
    GPIO_SetBits(ASW_VCC_PORT, ASW_VCC_PIN);

    /* Select default antenna --> A1 */
    asw_select(0);
}
/*---------------------------------------------------------------------------*/
void asw_select(uint8_t element)
{
    /* V3 */
    if(element & 0x01){
        GPIO_SetBits(ASW_V3_PORT, ASW_V3_PIN);
    }
    else{
        GPIO_ResetBits(ASW_V3_PORT, ASW_V3_PIN);
    }
    /* V2 */
    if(element & 0x02){
        GPIO_SetBits(ASW_V2_PORT, ASW_V2_PIN);
    }
    else{
        GPIO_ResetBits(ASW_V2_PORT, ASW_V2_PIN);
    }
    /* V1 */
    if(element & 0x04){
        GPIO_SetBits(ASW_V1_PORT, ASW_V1_PIN);
    }
    else{
        GPIO_ResetBits(ASW_V1_PORT, ASW_V1_PIN);
    }

    /* TODO: consider the use of GPIO_WriteBit(PORT, PIN, 0/1) */
}
