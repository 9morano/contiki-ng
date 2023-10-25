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
 *     Real-time timer for VESNA platform.
 *
 *     Contiki requires the following functions:
 *         - rtimer_arch_now(void)
 *         - rtimer_arch_init(void)
 *         - rtimer_arch_schedule(rtimer_clock_t t),
 *     and the following defines:
 *         - RTIMER_ARCH_SECOND
 *         - RTIMER_ARCH_DRIFT_PPM
 *
 * \author
 *      Grega Morano <grega.morano@ijs.si>
 *
 * \todo
 *      Timers & RTimers are usually CPU specific.. At some point, move this.
 */
/*---------------------------------------------------------------------------*/
#include "rtimer-arch.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "sys/critical.h"
/*---------------------------------------------------------------------------*/
#define RTIMER_TIMx TIM5
#define RTIMER_IRQn TIM5_IRQn
#define RTIMER_APB1 RCC_APB1Periph_TIM5
/*---------------------------------------------------------------------------*/
void rtimer_arch_init(void) {

    // TIM5 clock enable
    RCC_APB1PeriphClockCmd(RTIMER_APB1, ENABLE);

#if (VESNA_RTIMER_USE_EXTERNAL_SOURCE)
    TIM_TimeBaseInitTypeDef externalTimerInitStructure = {
        .TIM_Prescaler = 206 - 1,       // 0 = run @ 13.5MHz, 206 = run @ 65533.98 Hz
        .TIM_CounterMode = TIM_CounterMode_Up,
        .TIM_Period = 65533, // upper bound value of counter (65536) (same as RTIMER_ARCH_SECOND)
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0, // Not available on TIM5 anyway
    };

    GPIO_InitTypeDef gpioInitStructure = {
        .GPIO_Pin = GPIO_Pin_2,
        .GPIO_Mode = GPIO_Mode_IN_FLOATING,
        .GPIO_Speed = GPIO_Speed_10MHz,
    };
    // Initialize timer
    TIM_TimeBaseInit(RTIMER_TIMx, &externalTimerInitStructure);

        // GPIO clock enable
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // Do we need this? YES

    // Initialize GPIO
    GPIO_Init(GPIOA, &gpioInitStructure);

    // Connect channel 3 and 2 to channel 1 via XOR gate
    TIM_SelectHallSensor(RTIMER_TIMx, ENABLE);
    
    // Setup timer trigger as external clock source
    TIM_TIxExternalClockConfig(RTIMER_TIMx, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Falling, 0x0);
#else
    TIM_TimeBaseInitTypeDef timerInitStructure = {
        .TIM_Prescaler = 977 - 1,       // 0 = run @ 64MHz, 977 = run @ 65506 Hz
        .TIM_CounterMode = TIM_CounterMode_Up,
        .TIM_Period = 65503,            // upper bound value of counter (65506) (same as RTIMER_ARCH_SECOND)
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0,     // Not available on TIM5 anyway
    };
    // Initialize timer
    TIM_TimeBaseInit(RTIMER_TIMx, &timerInitStructure);
#endif

    // Set initial value (value could be random at power on)
    TIM_SetCounter(RTIMER_TIMx, 0);

    // Disable ALL interrupts from TIM5
    TIM_ITConfig(RTIMER_TIMx, 0xFF, DISABLE);

    // Clear interrupt bit, if it was triggered for some reason
    TIM_ClearITPendingBit(RTIMER_TIMx, TIM_IT_CC1);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTIMER_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable timer
    TIM_Cmd(RTIMER_TIMx, ENABLE);
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_now(void)
{   
    return (rtimer_clock_t)TIM_GetCounter(RTIMER_TIMx);
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
    int_master_status_t status;

    status = critical_enter();

    TIM_ITConfig(RTIMER_TIMx, TIM_IT_CC1, DISABLE);
    TIM_ClearITPendingBit(RTIMER_TIMx, TIM_IT_CC1);

    TIM_OCInitTypeDef tim_oc1_init;
    tim_oc1_init.TIM_OCMode = TIM_OCMode_Timing;
    tim_oc1_init.TIM_OCPolarity = TIM_OCPolarity_High;
    tim_oc1_init.TIM_Pulse = (uint16_t)t; // To be compared against
    tim_oc1_init.TIM_OutputState = TIM_OutputState_Disable;

    TIM_OC1Init(RTIMER_TIMx, &tim_oc1_init);
    TIM_ITConfig(RTIMER_TIMx, TIM_IT_CC1, ENABLE);

    critical_exit(status);
}
/*---------------------------------------------------------------------------*/
void
contiki_rtimer_isr(void)
{
    // Comparator 1 trigger
    if (TIM_GetITStatus(RTIMER_TIMx, TIM_IT_CC1) != RESET) {
        TIM_ITConfig(RTIMER_TIMx, TIM_IT_CC1, DISABLE);
        TIM_ClearITPendingBit(RTIMER_TIMx, TIM_IT_CC1);

        rtimer_run_next();
    }
}
