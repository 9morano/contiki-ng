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
 *        AT86RF2xx radio constants and defines. Required by the Contiki-NG.
 *        The file is included by contiki-conf.h
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#ifndef AT86RF2XX_DEF_H_
#define AT86RF2XX_DEF_H_
/*---------------------------------------------------------------------------*/
#define AT86RF2XX_MAX_FRAME_SIZE	(127)
#define AT86RF2XX_MIN_FRAME_SIZE	(3)
#define AT86RF2XX_CRC_SIZE			(2)
#define AT86RF2XX_LQI_SIZE			(1)
#define AT86RF2XX_MAX_PAYLOAD_SIZE	(AT86RF2XX_MAX_FRAME_SIZE - AT86RF2XX_CRC_SIZE)
/*---------------------------------------------------------------------------*/
/* Radio specific timings                                                    */
/*---------------------------------------------------------------------------*/
/* The delay between radio Tx request and SFD sent, in rtimer ticks */
#define AT86RF2XX_DELAY_BEFORE_TX		((unsigned)US_TO_RTIMERTICKS(180)) 
// Possible state transitions:
//      -> FORCE_TRX_OFF                    - 1us
//      TRX_OFF -> PLL_ON                   - 110us
//      RX_ON   -> PLL_ON                   - 1us
// Time before start sending
//      PLL_ON  -> BUSY_TX                  - 16us
// Time to send PREAMBLE + SFD (p.39)       
//      (4B + 1B) * 32 us/B                 - 160us 
//                                          = 286us

/* The delay between radio Rx request and start listening, in rtimer ticks */
#define AT86RF2XX_DELAY_BEFORE_RX		((unsigned)US_TO_RTIMERTICKS(160))
// Possible state transitions:
//       -> FORCE_TRX_OFF                   - 1us
//       TRX_OFF -> RX_ON                   - 110us 
//       PLL_ON -> RX_ON                    - 1us
// Time until PLL_LOCK should occur         - 32us
//                                          = 142us

/* The delay between the end of SFD reception and the radio returning 1 to 
 * receiving_packet() */
#define AT86RF2XX_DELAY_BEFORE_DETECT	((unsigned)US_TO_RTIMERTICKS(40))
// Time after SFD reception (p.39)
//       PHR reception                      - 32us
//       Interrupt latency                  - 9us
//                                          = 41us

/* The number of header and footer bytes of overhead at the PHY layer after 
 * SFD (1 length + 2 CRC) */
#define AT86RF2XX_PHY_OVERHEAD			(3) 

/* The air time for one byte in microsecond: 1 / (250kbps/8) == 32 us/byte */
#define AT86RF2XX_BYTE_AIR_TIME			(32) 

/*---------------------------------------------------------------------------*/
/* TSCH configuration according to / required by the AT86RF2xx radio         */
/*---------------------------------------------------------------------------*/
/* Radio won't go to off state between PACKET and ACK - faster transition */
#define TSCH_CONF_RADIO_ON_DURING_TIMESLOT  (1)

// TODO: AT86RF2XX_HW_FRAME_FILTERING not defined
#define TSCH_CONF_HW_FRAME_FILTERING        AT86RF2XX_HW_FRAME_FILTERING

/* The drift compared to "true" 10ms slots (see rtimer-arch.h). The drift is 
 * caused by the inaccurate r-timer */
#define TSCH_CONF_BASE_DRIFT_PPM            RTIMER_ARCH_DRIFT_PPM

/* TSCH timeslot timing of optimized for this driver implementation. The timings
 * are defined in at86rf2xx_tsch.c */
extern const uint16_t tsch_timeslot_timing_at86rf2xx_10000us_250kbps[];
#define AT86RF2XX_DEFAULT_TIMESLOT_TIMING	(tsch_timeslot_timing_at86rf2xx_10000us_250kbps)

/* Select the desired TSCH timings */
#define TSCH_CONF_DEFAULT_TIMESLOT_TIMING   AT86RF2XX_DEFAULT_TIMESLOT_TIMING


/*---------------------------------------------------------------------------*/
/* CSMA configuration according to / required by the AT86RF2xx radio         */
/*---------------------------------------------------------------------------*/
/* CSMA acknowledge configuration  */
#ifndef CSMA_CONF_SEND_SOFT_ACK
    #define CSMA_CONF_SEND_SOFT_ACK                (AT86RF2XX_SEND_SOFT_ACK)
    #define CSMA_CONF_ACK_WAIT_TIME                (RTIMER_SECOND / 300)
    #define CSMA_CONF_AFTER_ACK_DETECTED_WAIT_TIME (RTIMER_SECOND / 1200)
#endif

#endif /* AT86RF2XX_DEF_H_ */
