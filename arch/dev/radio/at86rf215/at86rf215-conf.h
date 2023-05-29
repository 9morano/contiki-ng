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
 *        Configuration file for the AT86RF215 radio.
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#ifndef AT86RF215_CONF_H_
#define AT86RF215_CONF_H_

#include "at86rf215-registermap.h"

/*---------------------------------------------------------------------------*/
/* Modulation specific defines                                               */
/*---------------------------------------------------------------------------*/

/* Here just for reference - could be used for config */
/* Maybe move to a different file (modulation utils or smth) */

/**
 * O-QPSK modulation
*/
enum
{
    CHIP_RATE_100,
    CHIP_RATE_200,
    CHIP_RATE_1000,
    CHIP_RATE_2000,
};
#define OQPSK_CHIP_RATE                 CHIP_RATE_2000

#define OQPSK_SYMBOL_DURATION_TABLE     (int[]){320, 160, 64, 64}
#define OQPSK_SYBOL_DURATION            OQPSK_SYMBOL_DURATION_TABLE[OQPSK_CHIP_RATE]

#define OQPSK_SYMBOL_LENGTH_TABLE       (int[]){32, 32, 64, 128}
#define OQPSK_SYMBOL_LENGTH             OQPSK_SYMBOL_LENGTH_TABLE[OQPSK_CHIP_RATE]

#define OQPSK_SHR_DURATION_TABLE        (int[]){28, 28, 72, 72}
#define OQPSK_SHR_DURATION              OQPSK_SHR_DURATION_TABLE[OQPSK_CHIP_RATE]

#define OQPSK_CCA_DURATION_TABLE        (int[]){4, 4, 8, 8}
#define OQPSK_CCA_DURATION              OQPSK_CCA_DURATION_TABLE[OQPSK_CHIP_RATE]
#define OQPSK_CCA_THRESHOLD             (-90) //dBm

// TODO: Depends on the band and regulation (Europe, US, Japan).
// Add config for different areas ...
#define OQPSK_CHANNEL_SPACING           5000000     // Hz
#define OQPSK_CHANNEL_CENTER_FREQ       2405000000  // Hz


/**
 * FSK modulation
*/


/**
 * OFDM modulation
*/



/*---------------------------------------------------------------------------*/
/* Radio configuration according to selected modulation and frequency        */
/*---------------------------------------------------------------------------*/

typedef struct {
    uint16_t address;
    uint8_t value;
} registerSetting_t;

/**
 * Current radio driver enables the selection of modulation and frequency band
 * only before compiling. In future we could reconfigure the drivers to support
 * adaptive modulation change according to the needs of the network ...
 */
#define FREQ_BAND_SUBGHZ    (0)
#define FREQ_BAND_24GHZ     (1)

#define MODULATION_OQPSK    (0)
#define MODULATION_FSK      (1)
#define MODULATION_OFDM     (2)

/**
 * Select the Frequency band to use: 0 = Sub-GHz, 1 = 2.4 GHz
 * 
 * NOTE: Current driver is designed for only one RF band at the time!
 */
#ifndef AT86RF215_CONF_FREQUENCY_BAND
#define AT86RF215_FREQUENCY_BAND        (FREQ_BAND_24GHZ)
#else
#define AT86RF215_FREQUENCY_BAND        (AT86RF215_CONF_FREQUENCY_BAND)
#endif

/**
 * Select the desired modulation
 */
#ifndef AT86RF215_CONF_MODULATION
#define AT86RF215_MODULATION            (MODULATION_OQPSK)
#else
#define AT86RF215_MODULATION            (AT86RF215_CONF_MODULATION)
#endif   


/**
 * Legacy O-QPSK (IEEE 802.15.4-2011) 
 *  - 250 kbit/s
 *  - currently only 2.4 GHZ band
 */

/* Set the channel spacing (example: 5 MHz / 25 kHz resolution) */
#define AT86RF215_RF_CHANNEL_SPACING     (uint8_t) (OQPSK_CHANNEL_SPACING / 25000)

/* Set the center frequency (example: (2405 MHz - 1.5 MHz offset) / 25 kHz) */
#define AT86RF215_RF_CENTER_FREQ_24GHZ_H (uint8_t) ((((OQPSK_CHANNEL_CENTER_FREQ - 1500000000) / 25000) & 0xFF00) >> 8)
#define AT86RF215_RF_CENTER_FREQ_24GHZ_L (uint8_t) (((OQPSK_CHANNEL_CENTER_FREQ - 1500000000) / 25000) & 0x00FF)

/* Legacy uses 16-bit CRC */
#define AT86RF215_CONF_CRC_16BIT        (1)
#define AT86RF215_CONF_CCA_THRESHOLD    (OQPSK_CCA_THRESHOLD)

static const registerSetting_t radio_config_oqpsk_250kbps_24ghz[] = 
{
    /* Enable BBC */
    {RG_BBC1_PC,         0x1F}, // CTX disabled, disable RX FCS filter, autogenerate TX FCS, 16-bit FCS, enable BBC, modulation: O-QPSK

    /* Baseband core config */
    {RG_BBC1_OQPSKPHRTX, 0x09},  // QPSK - legacy
    {RG_BBC1_OQPSKC0,    0x03},  // Chip freq.: 2000 kchips/s
    {RG_BBC1_OQPSKC1,    0x47},  // rx-override enabled, MINIMUM preamble-detection sensitivities,
    {RG_BBC1_OQPSKC2,    0x05},  // listen frames of legacy OQPSK, FCS is 16bit, power saving is off
    {RG_BBC1_OQPSKC3,    0x00},  // legacy OQPSK, search for SFD_1 only

    /* TX config */
    {RG_RF24_TXDFE,      0x81},  // TX cut-off freq. 1*fs/2 (default is 0.25*fs/2), TX sample rate 4000 kHz
    {RG_RF24_TXCUTC,     0x0B},  // AMP ramp up = 4us, LP cut-off freq. = 1000 KHz
    {RG_RF24_PAC,        0xFF},  // No current reduction, Max output power (0x1F)

    /* RX config */
    {RG_RF24_RXBWC,      0x0B},  // Rx filter BW 2000 kHz, IF 2000 kHz, no IF inversion or shift
    {RG_RF24_RXDFE,      0x41},  // RX cut-off freq. 0.5*fs/2 (default is 0.25*fs/2), RX sampling rate 4000 kHz
    {RG_RF24_EDC,        0x00},  // Automatic energy detection
    {RG_RF24_EDD,        0x13},  // averaging time T = DF * DTB --> DTB=128us, DF = 4 --> T = 512us
    {RG_RF24_AGCC,       0x01},  // Enable automatic gain control
    {RG_RF24_AGCS,       0x77},  // AGC target level = -30dBm

    /* PLL config */
    {RG_RF24_CS,         AT86RF215_RF_CHANNEL_SPACING},
    {RG_RF24_CCF0L,      AT86RF215_RF_CENTER_FREQ_24GHZ_L},
    {RG_RF24_CCF0H,      AT86RF215_RF_CENTER_FREQ_24GHZ_H},

};

#define AT86RF215_RADIO_CONFIGURATION           radio_config_oqpsk_250kbps_24ghz


/*---------------------------------------------------------------------------*/
/* Radio configuration according to selected MAC mode                        */
/*---------------------------------------------------------------------------*/

/**
 * If driver is used for CSMA (default)
 */
#if MAC_CONF_WITH_CSMA

    #define AT86RF215_CONF_FRAME_FILTER             (0) // Required for AACK, no need otherwise?
    #define AT86RF215_CONF_SEND_ON_CCA              (0)
    #define AT86RF215_CONF_POLL_MODE                (0)

    #define AT86RF215_CONF_AUTO_ACK                 (0) // Doesn't work, use SW ACK
    #define CSMA_CONF_SEND_SOFT_ACK                 (1)
    #define CSMA_CONF_ACK_WAIT_TIME                 (RTIMER_SECOND/100)
    #define CSMA_CONF_AFTER_ACK_DETECTED_WAIT_TIME  (RTIMER_SECOND / 1000)

    #define AT86RF215_CONF_AUTO_CRC                 (1)

#endif /* MAC_CONF_WITH_CSMA */


/**
 * If driver is used for TSCH
 */
#if MAC_CONF_WITH_TSCH

    #define AT86RF215_CONF_AUTO_ACK                 (0)
    #define AT86RF215_CONF_FRAME_FILTER             (0)
    #define AT86RF215_CONF_SEND_ON_CCA              (0)
    #define AT86RF215_CONF_POLL_MODE                (1)

    #define AT86RF215_CONF_AUTO_CRC                 (1)


    //#ifdef LOG_CONF_LEVEL_AT86RF215
    //#if LOG_CONF_LEVEL_AT86RF215 == LOG_LEVEL_DBG
    //#warning "TSCH will not work with DBG level"
    //#endif

#endif /* MAC_CONF_WITH_TSCH */



/*---------------------------------------------------------------------------*/
/* Radio configuration                                                       */
/*---------------------------------------------------------------------------*/

/**
 * Enable/disable auto-acknowledgement, where radio automatically sends
 * an ACK frame after receiving a frame with the ACK request bit set
 */
#ifndef AT86RF215_CONF_AUTO_ACK
#define AT86RF215_AUTO_ACK              (0)
#else
#define AT86RF215_AUTO_ACK              (AT86RF215_CONF_AUTO_ACK)
#endif

/**
 * Enable/disable address filter mode, where radio only accepts frames
 * with matching address
 */
#ifndef AT86RF215_CONF_FRAME_FILTER
#define AT86RF215_FRAME_FILTER          (0)
#else
#define AT86RF215_FRAME_FILTER          (AT86RF215_CONF_FRAME_FILTER)
#endif

/**
 * Do a CCA manually before sending a packet
 */ 
#ifndef AT86RF215_CONF_SEND_ON_CCA
#define AT86RF215_SEND_ON_CCA           (0)
#else
#define AT86RF215_SEND_ON_CCA           (AT86RF215_CONF_SEND_ON_CCA)
#endif

/**
 * Radio does a CCA automaticaly before sending a packet
 *
 * TODO: Not implemented yet! 
 * (No need, only if we want to offload the operation to the radio)
 */
#ifndef AT86RF215_CONF_AUTO_CCATX
#define AT86RF215_AUTO_CCATX            (0)
#else
#define AT86RF215_AUTO_CCATX            (AT86RF215_CONF_AUTO_CCATX)
#endif

/**
 * CCA threshold in dBm
 */
#ifndef AT86RF215_CONF_CCA_THRESHOLD
#define AT86RF215_CCA_THRESHOLD         (-70)
#else
#define AT86RF215_CCA_THRESHOLD         (AT86RF215_CONF_CCA_THRESHOLD)
#endif

/**
 * Radio poll mode - should disable interrupts and enable upper levels
 * to poll the radio for events.
 */
#ifndef AT86RF215_CONF_POLL_MODE
#define AT86RF215_POLL_MODE             (1)
#else
#define AT86RF215_POLL_MODE             (AT86RF215_CONF_POLL_MODE)
#endif

/**
 * Enable/disable auto-CRC generation
 * Radio will automatically append CRC to outgoing frames
 */
#ifndef AT86RF215_CONF_AUTO_CRC
#define AT86RF215_AUTO_CRC              (1)
#else
#define AT86RF215_AUTO_CRC              (AT86RF215_CONF_AUTO_CRC)
#endif

/**
 * Select either 16-bit (1) or 32-bit (0) CRC
*/
#ifndef AT86RF215_CONF_CRC_16BIT
#define AT86RF215_CRC_16BIT             (1)
#else
#define AT86RF215_CRC_16BIT             (AT86RF215_CONF_CRC_16BIT)
#endif

/**
 * Enable the automatic transition to RX state after a packet is sent.
 *
 * TODO: Not implemented yet (No need?)
 */
#ifndef AT86RF215_CONF_AUTO_TX2RX
#define AT86RF215_AUTO_TX2RX            (0)
#else
#define AT86RF215_AUTO_TX2RX            (AT86RF215_CONF_AUTO_TX2RX)
#endif

/**
 * Debug driver operation with GPIO pins (development purpose only)
 */
#ifndef AT86RF215_CONF_GPIO_DEBUG_TSCH
#define AT86RF215_DEBUG_GPIO_TSCH       (0)
#else
#define AT86RF215_DEBUG_GPIO_TSCH       (AT86RF215_CONF_GPIO_DEBUG_TSCH)
#endif

/**
 * Logging level for the AT86RF215 driver.
 */
#ifndef LOG_CONF_LEVEL_AT86RF215
#define AT86RF215_LOG_LEVEL             (LOG_LEVEL_WARN)
#else
#define AT86RF215_LOG_LEVEL             (LOG_CONF_LEVEL_AT86RF215)
#endif


#endif /* AT86RF215_CONF_H_ */