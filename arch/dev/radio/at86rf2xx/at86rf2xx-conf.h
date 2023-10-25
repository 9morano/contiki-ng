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
 *        AT86RF2xx radio driver configuration file.

 * \author
 *        Grega Morano <grega.morano@ijs.si>
 *        Gregor Cerar <gregor.cerar@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#ifndef AT86RF2XX_CONF_H_
#define AT86RF2XX_CONF_H_
/*---------------------------------------------------------------------------*/
/* Configure the operation of radio driver                                   */
/*---------------------------------------------------------------------------*/
/* Driver's default log level */ 
#ifndef LOG_CONF_LEVEL_AT86RF2XX
#define LOG_LEVEL_AT86RF2XX     (LOG_LEVEL_WARN)
#else
#define LOG_LEVEL_AT86RF2XX     (LOG_CONF_LEVEL_AT86RF2XX)
#endif

/* Collect packet statistics */
#ifndef AT86RF2XX_CONF_PACKET_STATS
#define AT86RF2XX_PACKET_STATS     (0)
#else
#define AT86RF2XX_PACKET_STATS     (AT86RF2XX_CONF_PACKET_STATS)
#endif

/* Collect statistics on radio driver operations */
#ifndef  AT86RF2XX_CONF_DRIVER_STATS
#define AT86RF2XX_DRIVER_STATS      (0)
#else
#define AT86RF2XX_DRIVER_STATS      (AT86RF2XX_CONF_DRIVER_STATS)
#endif

/*---------------------------------------------------------------------------*/
/* Configure the driver according to the selected MAC mode                   */
/*---------------------------------------------------------------------------*/

/* If driver is built for Contiki's 6Tisch implementation */
#if MAC_CONF_WITH_TSCH
    #define AT86RF2XX_CONF_AACK                     (0)
    #define AT86RF2XX_CONF_ARET                     (0)
    //#define AT86RF2XX_CONF_HW_CCA                 (0)
    #define AT86RF2XX_CONF_POLLING_MODE             (1)
#endif


/*---------------------------------------------------------------------------*/
/* Configure the radio                                                       */
/*---------------------------------------------------------------------------*/

// Enable radio's auto acknowledge capabilities (extended mode)
#ifndef AT86RF2XX_CONF_AACK
#define AT86RF2XX_AACK   (0)
#else
#define AT86RF2XX_AACK   (AT86RF2XX_CONF_AACK)
#endif

// Enable radio's auto retransmission capabilities (extended mode)
#ifndef AT86RF2XX_CONF_ARET
#define AT86RF2XX_ARET  (0)
#else
#define AT86RF2XX_ARET  (AT86RF2XX_CONF_ARET)
#endif

// Enables radio's automatic CCA before sending
#define AT86RF2XX_HW_CCA   (AT86RF2XX_ARET)

/* CSMA acknowledge configuration  */
#define AT86RF2XX_SEND_SOFT_ACK (!AT86RF2XX_AACK)

// Number of CSMA retries 0-5, 6 = reserved, 7 = immediately without CSMA/CA
#ifndef AT86RF2XX_CONF_CSMA_RETRIES
#define AT86RF2XX_CSMA_RETRIES  (5)
#else
#if AT86RF2XX_CONF_CSMA_RETRIES < 0 || AT86RF2XX_CONF_CSMA_RETRIES > 5
#error "Invalid AT86RF2XX_CONF_CSMA_RETRIES"
#endif
#define AT86RF2XX_CSMA_RETRIES  (AT86RF2XX_CONF_CSMA_RETRIES)
#endif

// Number of frame retries, if no ACK, 0-15 (TX_ARET-only)
#ifndef AT86RF2XX_CONF_FRAME_RETRIES
#define AT86RF2XX_FRAME_RETRIES     (15)
#else
#define AT86RF2XX_FRAME_RETRIES     (AT86RF2XX_CONF_FRAME_RETRIES)
#endif

// Enable offloading checksum calculation to the radio chip
#ifndef AT86RF2XX_CONF_CHECKSUM
#define AT86RF2XX_CHECKSUM  (1)
#else
#define AT86RF2XX_CHECKSUM  (AT86RF2XX_CONF_CHECKSUM)
#endif

// Skip radio's address filter (AACK only)
#ifndef AT86RF2XX_CONF_PROMISCOUS_MODE
#define AT86RF2XX_PROMISCOUS_MODE   (0)
#else
#define AT86RF2XX_PROMISCOUS_MODE   (AT86RF2XX_CONF_PROMISCOUS_MODE)
#endif

// Configure the radio driver into pooling mode
#ifndef AT86RF2XX_CONF_POLLING_MODE
#define AT86RF2XX_POLLING_MODE   (0)
#else
#define AT86RF2XX_POLLING_MODE   (AT86RF2XX_CONF_POLLING_MODE)
#endif

// Calibration period duration in seconds (~4min)
#ifndef AT86RF2XX_CONF_CALIBRATION_PERIOD
#define AT86RF2XX_CALIBRATION_PERIOD   (240)
#else
#define AT86RF2XX_CALIBRATION_PERIOD   (AT86RF2XX_CONF_CALIBRATION_PERIOD)
#endif

/**
 * Enable CLKM output - can be used as external clock source for the uC
 * This can eliminate Sampling Frequency Offset (SFO) during PMU 
 */
#ifndef AT86RF2XX_CONF_ENABLE_CLKM
#define AT86RF2XX_ENABLE_CLKM   (0)
#else
#define AT86RF2XX_ENABLE_CLKM   (AT86RF2XX_CONF_ENABLE_CLKM)
#endif


#endif /* AT86RF2XX_CONF_H_ */
