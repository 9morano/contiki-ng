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
 *        Header file for the at86rf215.c
 * \author
 *        Grega Morano <grega.morano@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#ifndef AT86RF215_H_
#define AT86RF215_H_


// Simplify decode radio states/flags
typedef union {
	struct {
		// Corresponds to radio states
		uint8_t RECEIVE_ON:1;	// Indicates that radio is (was) in RX state - it is used manly for CSMA, so that the radio is always on except when it is set to be off with _off();
		uint8_t PLL_LOCK:1; 	// Indicates when TXPREP state was reached
		uint8_t PLL_ERR:1;	
		uint8_t PACKET_PEN:1;	// Packet is pending in the radios buffer

		// Corresponds to radio interrupts
		uint8_t RX_START:1; 	// Indicates when frame was detected
		uint8_t AMI:1;			// Indicates when frame address matched
		uint8_t TX_END:1;		// Indicates TX operation completed
		uint8_t CCA:1;			// Indicates when CCA has ended (single Energy Detection has Completed)
	};
	uint8_t value;
} at86rf215_flags_t;

// Simplify decode IRQ bits
typedef union {
	struct {
		uint8_t IRQ1_WAKEUP:1;	// If radio came from SLEEP state
		uint8_t IRQ2_TRXRDY:1;	// Indicates lock of the PLL (or state change from TRCOFF to TXPREP)
		uint8_t IRQ3_EDC:1;		// End of manual measurement of Energy Detection
		uint8_t IRQ4_BATLOW:1;

		uint8_t IRQ5_TRXERR:1;	// If error during TRX (PLL-unlocks)
		uint8_t IRQ6_IQIFSF:1;	// Applicable in I/Q radio mode only.
	};
	uint8_t value;
} at86rf215_rf_irq_t;

typedef union {
	struct {
		uint8_t IRQ1_RXFS:1;	// If a valid PHY header is received
		uint8_t IRQ2_RXFE:1;	// Issued at the end of a successfull frame reception (first level filtering must pass)
		uint8_t IRQ3_RXAM:1;	// If address filter is enabled and frame is extended
		uint8_t IRQ4_RXEM:1;	// If address recognised as matching
		
		uint8_t IRQ5_TXFE:1;	// When the frame is transmitted (at the end)
		uint8_t IRQ6_AGCH:1;	// used by AGC (detected SHR)
		uint8_t IRQ7_AGCR:1;	// used by AGC (end of RX)
		uint8_t IRQ8_FBLI:1;	// If the preprogramed number of octects is received in the FB
	};
	uint8_t value;
} at86rf215_bbc_irq_t;



typedef struct {
    //uint8_t content[AT86RF215_MAX_PAYLOAD_SIZE];
    //uint8_t len;
    //uint16_t crc;
    uint8_t lqi;
    int8_t rssi;
	rtimer_clock_t timestamp;
} rxFrame_t;

/*
// No need to store the frame
typedef struct {
    uint8_t content[AT86RF215_MAX_PAYLOAD_SIZE];
    uint8_t len;
    uint16_t *crc;
} txFrame_t;
*/

// Used for basic radio configuration
typedef struct {
    uint16_t address;
    uint8_t value;
} registerSetting_t;






/*---------------------------------------------------------------------------*/
/* NETSTACK API radio driver functions */
/*---------------------------------------------------------------------------*/
/** The NETSTACK data structure for the AT86RF215 driver */
extern const struct radio_driver at86rf215_driver;
/*---------------------------------------------------------------------------*/
int at86rf215_init(void);
int at86rf215_prepare(const void *payload, unsigned short payload_len);
int at86rf215_transmit(unsigned short payload_len);
int at86rf215_send(const void *payload, unsigned short payload_len);
int at86rf215_read(void *buf, unsigned short buf_len);
int at86rf215_receiving_packet(void);
int at86rf215_pending_packet(void);
int at86rf215_on(void);
int at86rf215_off(void);
int at86rf215_channel_clear(void);
static radio_result_t at86rf215_get_value(radio_param_t param, radio_value_t *value);
static radio_result_t at86rf215_set_value(radio_param_t param, radio_value_t value);
static radio_result_t at86rf215_get_object(radio_param_t param, void *dest, size_t size);
static radio_result_t at86rf215_set_object(radio_param_t param, const void *src, size_t size);




/*---------------------------------------------------------------------------*/
/* Private functions */
/*---------------------------------------------------------------------------*/

/**
 * \brief Set auto ack mode.
 * \param enable 1 to enable auto ack, 0 to disable.
 *
 * \note Current driver supports only TSCH MAC mode - Auto ACK is not used in TSCH.
 */
static void set_auto_ack(uint8_t enable);

/**
 * \brief Set frame filtering mode.
 * \param enable 1 to enable frame filtering, 0 to disable.
 *
 * \note Current driver supports only TSCH MAC mode - Frame address filtering is not used in TSCH.
 */
static void set_frame_filtering(uint8_t enable);

/**
 * \brief Put the radio into pool mode - disable interrupts.
 * \param enable 1 to enable poll mode, 0 to disable.
 *
 * In poll mode, radio interrupts should be disabled, and the radio driver
 * never calls upper layers. TSCH will poll the driver for incoming packets.
 * 
 * However, in this implementation, the drivers requires interrupts to be
 * enabled to detect incoming packets and correctly obtain timestamps.
 */
static void set_poll_mode(uint8_t enable);

/**
 * \brief Do a CCA as described in reference manual p. 172.
 * 
 * \note TODO: not tested yet
 */
static void set_send_on_cca(uint8_t enable);

/**
 * \brief Set the CCA threshold.
 * \param threshold The CCA threshold in dBm.
 */
static void set_cca_threshold(uint8_t threshold);

/**
 * \brief Get the CCA threshold.
 * \return The CCA threshold in dBm.
 *
 * \note Current driver supports only TSCH MAC mode - CCA threshold is not used in TSCH.
 */
static uint8_t get_cca_threshold(void);

/**
 * \brief Store the PAN ID to the radio.
 * \param pan The PAN ID.
 * 
 * The radio will only accept packets with matching PAN ID and address.
 * The address are stored in the unit #0!
 */
void set_pan_ID(uint16_t pan);

/**
 * \brief Get the PAN ID of the radio.
 * \return The PAN ID.
*/
uint16_t get_pan_ID(void);

/**
 * \brief Store the 16-bit address to the radio.
 * \param addr The 16-bit address.
*/
void set_short_addr(uint16_t addr);

/**
 * \brief Get the 16-bit address of the radio.
 * \return The 16-bit address.
*/
uint16_t get_short_addr(void);

/**
 * \brief Store the 64-bit address to the radio.
 * \param addr Pointer to 64-bit address.
 * \param len Length of the address (must not exceed 8).
*/
void set_long_addr(const uint8_t *addr, uint8_t len);

/**
 * \brief Get the 64-bit address of the radio.
 * \param addr Pointer to 64-bit address.
 * \param len Length of the address.
*/
void get_long_addr(uint8_t *addr, uint8_t len);

/**
 * \brief Set TX power in dBm.
 * \param power The desired TX power in dBm.
 *
 * Radio accepts values from 0 to 31 (0x1F) which translates into range
 * -15 dBm to 15 dBm (depending on the modulation used).
 */
static void set_tx_power(int8_t power);

/**
 * \brief Get TX power in dBm.
 * \return The current TX power in dBm.
 */
static int8_t get_tx_power(void);

/**
 * \brief Set the frequency channel.
 * \param channel The desired frequency channel.
 *
 * \note Current drivers are tested only on 2.4 GHz ISM band.
 *
 * Radio supports various setups, but this function is used to set up the
 * channels according to IEEE 802.15.4 - TSCH. Valid channels are 11 to 26.
 *      f = 2405 + 5 * (ch - 11) MHz
 */
static void set_channel(uint8_t channel);

/**
 * \brief Get the frequency channel.
 * \return The current frequency channel.
 *
 * Easier to store it than to read it from the radio...
 */
static uint8_t get_channel(void);

/**
 * \brief Set the exact PLL frequency.
 * \param freq The desired PLL frequency.
 * \param decimal The decimal part of the frequency.
 * 
 * \note While receiving, the PLL frequency should be for IF smaller than the
 * freq. you want to listen to...
*/
void set_frequency(uint16_t freq, uint8_t decimal);



/*---------------------------------------------------------------------------*/
/* HAL */
/*---------------------------------------------------------------------------*/

/**
 * \brief      Read a single register from the AT86RF215
 *
 * \param addr The address of the register to read (16-bit)
 * \return     The value of the register
 *
 * Implementation of so called single access mode
 */
uint8_t regRead(uint16_t address);

/**
 * \brief      Write a register to the AT86RF215
 *
 * \param addr The address of the register to write (16-bit)
 * \param data The value to write to the register
 */
void regWrite(uint16_t address, uint8_t data);

/**
 * \brief      Read one ore multiple bits from the radio's register
 * 
 * \param      (addr, mask, offset) --> u can use SR_ defines in registermap.h
 * \return     The value of the bits, shifted by offset 
 */
uint8_t bitRead(uint16_t address, uint8_t mask, uint8_t offset);

/**
 * \brief      Write one ore multiple bits to the radio's register
 * 
 * \param      (addr, mask, offset) --> u can use SR_ defines in registermap.h
 * \param value the value to write to the bits, shifted by offset
 * 
 * The content of the registers is not overwritten.
*/
void bitWrite(uint16_t address, uint8_t mask, uint8_t offset, uint8_t value);

/**
 * \brief      Read a burst of data from the AT86RF215 registers.
 *
 * \param addr The address of the first register to read (16-bit)
 * \param data Pointer to the buffer where the data will be stored
 * \param len  The number of bytes to read
 *
 * Implementation of so called block access mode.
 */
void burstRead(uint16_t address, uint8_t *data, uint16_t len);
/**
 * \brief      Write a burst of data to the AT86RF215 registers.
 * 
 * \param addr The address of the first register to write (16-bit)
 * \param data Pointer to the buffer of the data to write
 * \param len  The number of bytes to write
 * 
 * Implementation of so called block access mode.
 */
void burstWrite(uint16_t address, uint8_t *data, uint16_t len);

/**
 * \brief      Costum fuction to read a packet frame from the radio's memory
 * 
 * \param frame Pointer to the buffer where the data will be stored
 * \param frame_len variable where the length of the frame will be stored
*/
void frameRead(uint8_t *frame, uint16_t *frame_len);

/**
 * \brief      Costum fuction to write a packet frame to the radio's memory
 * 
 * \param frame Pointer to the buffer of the data to write
 * \param frame_len the length of the frame
*/
void frameWrite(uint8_t *frame, uint16_t frame_len);



#endif /* AT86RF215_H_ */