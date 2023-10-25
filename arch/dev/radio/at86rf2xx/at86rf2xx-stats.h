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
 *        Functions to collect and store the packet statistics. Can be used to 
 *  	  monitor the network on low level of operation.
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#ifndef AT86RF2XX_STATS_H_
#define AT86RF2XX_STATS_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/log.h"
#include "at86rf2xx.h"
#include "os/net/mac/framer/frame802154.h"
/*---------------------------------------------------------------------------*/
/* Define the size of ring-buffers */
#define AT86RF2XX_STATS_RINGBUF_SIZE    	(20)
#define AT86RF2XX_STATS_RINGBUF_NOISE_SIZE 	(1001)

/*---------------------------------------------------------------------------*/
/*     PACKET STATISTICS													 */
/*---------------------------------------------------------------------------*/
typedef struct {
    struct {
		uint32_t s;
		uint32_t us;
	} ts;				// Timestamp

    frame802154_t frame; // SQN, UC/BC, ADDRESS,... 

    uint16_t 	count;
    uint8_t 	channel;
    uint8_t 	power;
    uint8_t 	broadcast;
} txPacket_t;

typedef struct {
    struct {
		uint32_t s;
		uint32_t us;
	} ts;

    frame802154_t frame;

    uint16_t count;
    uint8_t  channel;
    int8_t   rssi;
    uint8_t  lqi;
} rxPacket_t;

typedef struct {
	txPacket_t items[AT86RF2XX_STATS_RINGBUF_SIZE];
	uint8_t head;
    uint8_t tail;
} packet_ringbuf_t;

void STATS_initBuff(void);
void STATS_rxPush(rxFrame_t *raw);
int  STATS_rxPull(rxPacket_t *item);
void STATS_txPush(txFrame_t *raw);
int  STATS_txPull(txPacket_t *item);

void STATS_parse_rxFrame(rxFrame_t *raw, rxPacket_t *out);
void STATS_parse_txFrame(txFrame_t *raw, txPacket_t *out);

void STATS_print_packet_stats(void);
void STATS_clear_packet_stats(void);

/*---------------------------------------------------------------------------*/
/*     BACKGROUND NOISE 													 */
/*---------------------------------------------------------------------------*/
typedef struct {
	struct {
		uint32_t s;
		uint32_t us;
	} ts;

	uint8_t channel;
	int8_t rssi; // received signal strength index (dBm)
} bgNoise_t;


typedef struct {
	bgNoise_t items[AT86RF2XX_STATS_RINGBUF_NOISE_SIZE];
	uint16_t head;
    uint16_t tail;
} bgn_ringbuf_t;


void STATS_noisePush(const bgNoise_t *noise);
int  STATS_noisePull(bgNoise_t *noise);

void STATS_update_background_noise(void);

void STATS_print_background_noise(void);
void STATS_clear_background_noise(void);

/*---------------------------------------------------------------------------*/
/*     DRIVER STATISTICS													 */
/*---------------------------------------------------------------------------*/
enum {
	rxDetected,		// Detected packets
	rxSuccess,		// Successfully received packets
	rxToStack,		// Not used in TSCH
	rxAddrMatch,	// Not used in TSCH?

	rxData,			// Received Data packet
	rxBeacon,		// Received Beacon packet
	rxAck,			// Received Acknowledge

	txCollision,	// Not used in TSCH? - channel access failure
	txNoAck,		// Not used in TSCH - no ACK received
	txSuccess,		// Not used in TSCH - it's same as txCount
	txCount,		// Num of sent packets
	txError,		// Num of errors
	txTry,			// Num of TX tries

	txAck,			// Transmitted Ack
	txBeacon,		// Transmitted Beacon
	txData,			// Transmitted Data
	txReqAck,		// Request ACK (unicast packet)

	AT86RF2XX_STATS_COUNT
};

	void STATS_display_driver_stats_inline(void);
	void STATS_display_driver_stats(void);
	void STATS_print_driver_stats(void);

#if AT86RF2XX_DRIVER_STATS
	extern volatile uint32_t at86rf2xxStats[AT86RF2XX_STATS_COUNT];
	#define AT86RF2XX_STATS_GET(event)		at86rf2xxStats[event]
	#define AT86RF2XX_STATS_ADD(event)		at86rf2xxStats[event]++
	#define AT86RF2XX_STATS_RESET()    		memset(at86rf2xxStats, 0, sizeof(at86rf2xxStats[0]) * AT86RF2XX_STATS_COUNT)
#else
	#define AT86RF2XX_STATS_GET(event)		(0)
	#define AT86RF2XX_STATS_ADD(event)
	#define AT86RF2XX_STATS_RESET()
#endif

#endif /* AT86RF2XX_STATS_H_ */
