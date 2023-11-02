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
 * \author
 *      Grega Morano <grega.morano@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_
#include "sys/rtimer.h"
#include "vesna-conf.h"
/*---------------------------------------------------------------------------*/
/**
 * Contiki(-ng) uses several timers. We implemented rtimer (r is for real-time) using TIM5 general purpose 
 * timer. Rtimer is typically used for TSCH operations, thus it should be precise.
 * 
 * TIM5 properties:
 *     - 16-bit up/down counter,
 *     - interrupt can be triggered on overflow or specific value,
 * 
 * Our goal is to get >= 32kHz triggers (see tsch-slot-operation.c). We set goal to have 64kHz triggers.
 * 
 * Internal clock of STM32 is drifting a lot - too much for precise TSCH operations. This presents a 
 * problem, because our devices are missing the slots. So we have 2 options:
 * 
 * 1) When we are using SNR board, we can use AT86RF2xx oscillator clock as main clock for STM32, which has
 *     very low drift (configured in platform.c).
 *     Usage: VESNA_CONF_USE_EXTERNAL_CLOCK  (1)
 * 
 * 2) When we are using ISMTV board, AT86RF2xx CLK pin is not connected anywhere. But we can use oscillator
 *     of CC1101 radio, which is connected to TIM5 Channel 3 (PA2 on STM32). So only TIM5 will have external 
 *     clock source, which is not drifting.
 *     Usage: VESNA_CONF_RTIMER_USE_EXTERNAL_SOURCE   (1)
 */
/*---------------------------------------------------------------------------*/
/*
If radio is on ISMTV board: rtimer has 65.533 kHz --> RTIMER_ARCH_SECOND = 65533
Drift calculation:
    Slot length 10000 usec, which gives us 655 ticks (from macro US_TO_RTIMERTICKS)
    Tick duration: 15.2594875864 us
    Real slot duration is then : 9994.96436 usec
    Target - real duration = 5.03564 us
    RTIMER_ARCH_DRIFT_PPM = 504
 
If radio is not on ISMTV board: rtimer has 65.503 kHz --> RTIMER_ARCH_SECOND = 65503
Drift calculation:
    Slot length 10000 usec, which gives us 655 ticks (from macro US_TO_RTIMERTICKS)
    Tick duration: 15.2664763445 us
    Real slot duration is then : 9999.54200 usec
    Target - real duration = 0.4580 us
    RTIMER_ARCH_DRIFT_PPM = 46
 */
/*---------------------------------------------------------------------------*/

/* When VESNA is using TSCH, one of the workarrounds must be selected ... SNR
 * and SNE_ISMTV can use only one of the option, while SNE_ATASW has the 
 * ability to use either one, but most likely not both at the same time. We do 
 * the setup check here. 
 */
#if MAC_CONF_WITH_TSCH
#if (VESNA_RTIMER_USE_EXTERNAL_SOURCE)
    #if BOARD_SNR
        #error "SNR board has no option to feed the rtimer from external source."
    #endif

    #if(VESNA_USE_EXTERNAL_CLOCK)
        #error "rtimer config: both are 1"
    #else
        #define RTIMER_ARCH_SECOND      (65533)
        #define RTIMER_ARCH_DRIFT_PPM   (503)
    #endif
#else
    #if (BOARD_SNE_ISMTV_V1_0 || BOARD_SNE_ISMTV_V1_1)
        #error "SNE_ISMTV boards have no option to feed the HSE clock from external source."
    #endif

    #if(VESNA_USE_EXTERNAL_CLOCK)
        #define RTIMER_ARCH_SECOND		(65503)
        #define RTIMER_ARCH_DRIFT_PPM   (46)
    #else
        #error "rtimer config: bot are 0"
    #endif
#endif
#endif

/*---------------------------------------------------------------------------*/
/* Converts micro seconds to rtimer ticks (65536 ticks/s ---> 1us = 0.065536 tick) */
// Eqn.: T = (us * 0.065536) +- 1/2
#define US_TO_RTIMERTICKS(us)   ((us) >= 0 ? \
                                (uint32_t)(((((int64_t)(us)) * (RTIMER_ARCH_SECOND)) + 500000) / 1000000L) : \
                                (uint32_t)(((((int64_t)(us)) * (RTIMER_ARCH_SECOND)) - 500000) / 1000000L)) 

/*---------------------------------------------------------------------------*/
/* Converts rtimer ticks to micro seconds (1/65536 Hz ---> 1 tick ~ 15.25879 us) */
// Eqn.: us = (T * 15.258) +- 1/2
#define RTIMERTICKS_TO_US(t)    ((t) >= 0? \
                                ((((int32_t)(t)) * 1000000L + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)) : \
                                ((((int32_t)(t)) * 1000000L - ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)))

/*---------------------------------------------------------------------------*/
/* Converts rtimer ticks to micro-seconds (64-bit version) because the 32-bit one cannot handle T >= 4294 ticks.*/
/* Intended only for positive values of t */
#define RTIMERTICKS_TO_US_64(t) ((uint32_t)(((int64_t)(t)) * 1000000L + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)) 

/*---------------------------------------------------------------------------*/
/**
 * \brief ISR for timer interrupt
 */
void contiki_rtimer_isr(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief return the current timer count
 */
rtimer_clock_t rtimer_arch_now(void);

#endif
