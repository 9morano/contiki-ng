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
 *		  Gregor Cerar <gregor.cerar@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#ifndef AT86RF2XX_H_
#define AT86RF2XX_H_
/*---------------------------------------------------------------------------*/
/* Radio identification - drivers support different radios
 * Manfacturer ID is same for all of them == 0x00 0x1F
 */
#define AT86RF2XX_MAN_ID_0		((uint8_t)0x1F)
#define AT86RF2XX_MAN_ID_1		((uint8_t)0x00)

#define AT86RF2XX_ID_UNDEFINED  ((uint8_t)0x00)

#if AT86RF212
	#define AT86RF2XX_ID     	((uint8_t)0x07)
	#define AT86RSSI_BASE_VAL	((int8_t)-91)

#elif AT86RF230				// was never tested
	#define AT86RF2XX_ID     	((uint8_t)0x02) 
	#define AT86RSSI_BASE_VAL	((int8_t)-91)

#elif AT86RF231
	#define AT86RF2XX_ID     	((uint8_t)0x03)
	#define AT86RSSI_BASE_VAL	((int8_t)-91)

#elif AT86RF232				// was never tested
	#define AT86RF2XX_ID     	((uint8_t)0x0A)
	#define AT86RSSI_BASE_VAL	((int8_t)-91)

#elif AT86RF233
	#define AT86RF2XX_ID     	((uint8_t)0x0B)
	#define AT86RSSI_BASE_VAL	((int8_t)-94)
#endif



/*---------------------------------------------------------------------------*/
typedef union {
	struct {
		uint8_t SLEEP:1;	// Indicate radio sleep state
		uint8_t PLL_LOCK:1; // Indicate when TX/RX state was reached
		uint8_t RX_START:1; // Indicate when frame was detected
		uint8_t AMI:1;		// Indicate when frame address matched
		uint8_t TRX_END:1;	// Indicate TX/RX operation completed
		uint8_t TRX_UR:1;	// Indicate TX/RX buffer access violation
		uint8_t CCA:1;		// Indicate when CCA is in progress
	};
	uint8_t value;
} at86rf2xx_flags_t;

/*---------------------------------------------------------------------------*/
// Simplify decode of AT86RF2xx IRQ bits
typedef union {
	struct {
		uint8_t IRQ0_PLL_LOCK:1;
		uint8_t IRQ1_PLL_UNLOCK:1;
		uint8_t IRQ2_RX_START:1;
		uint8_t IRQ3_TRX_END:1;
		uint8_t IRQ4_AWAKE_END:1;
		uint8_t IRQ5_AMI:1;
		uint8_t IRQ6_TRX_UR:1;
		uint8_t IRQ7_BAT_LOW:1;
	};
	uint8_t value;
} at86rf2xx_irq_t;

/*---------------------------------------------------------------------------*/
typedef struct {
    uint8_t content[AT86RF2XX_MAX_FRAME_SIZE];
    uint8_t len;
    uint16_t *crc;
    uint8_t lqi;
    int8_t rssi;
	uint8_t trac;
#if AT86RF233
	uint8_t ed;
	uint8_t rx_status;
#endif
	rtimer_clock_t timestamp;
} rxFrame_t;

/*---------------------------------------------------------------------------*/
typedef struct {
    uint8_t content[AT86RF2XX_MAX_FRAME_SIZE];
    uint8_t len;
    uint16_t *crc;
	uint8_t trac;
} txFrame_t;



/*---------------------------------------------------------------------------*/
/* Radio driver API															 */
/*---------------------------------------------------------------------------*/
int at86rf2xx_init(void);
int at86rf2xx_prepare(const void *payload, unsigned short payload_len);
int at86rf2xx_transmit(unsigned short payload_len);
int at86rf2xx_send(const void *payload, unsigned short payload_len);
int at86rf2xx_read(void *buf, unsigned short buf_len);
int at86rf2xx_cca(void);
int at86rf2xx_receiving_packet(void);
int at86rf2xx_pending_packet(void);
int at86rf2xx_on(void);
int at86rf2xx_off(void);

#endif /* AT86RF2XX_H_ */
