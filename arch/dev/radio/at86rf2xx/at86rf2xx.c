/**
 * Radio drivers for AT86RF212, AT86RF231 and AT86RF233. 
 */

#include <stdio.h>
#include <string.h>

#include <lib/crc16.h>

#include "contiki.h"
#include "sys/log.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "sys/energest.h"
#include "sys/rtimer.h"

#include "sys/critical.h"

#include "at86rf2xx-conf.h"
#include "at86rf2xx-arch.h"
#include "at86rf2xx-registermap.h"
#include "at86rf2xx.h"
#include "at86rf2xx-stats.h"

#define LOG_MODULE  "at86rf2xx"
#define LOG_LEVEL   LOG_LEVEL_AT86RF2XX

PROCESS(at86rf2xx_process, "AT86RF2xx driver");

#if AT86RF2XX_DRIVER_STATS
volatile uint32_t at86rf2xxStats[AT86RF2XX_STATS_COUNT] = { 0 };
#endif

// SRC: https://barrgroup.com/Embedded-Systems/How-To/Define-Assert-Macro
#define ASSERT(expr) ({ if (!(expr)) LOG_ERR("Err: " #expr "\n"); })
#define ASSERT_EQUAL(x, y)  ({ if (x != y) LOG_ERR("Err: " #x " != " #y ", %i != %i\n", x, y); })


#define BUSYWAIT_UNTIL(expr)    ({ while(!(expr)); })


#define DEFAULT_IRQ_MASK    (IRQ2_RX_START | IRQ3_TRX_END | IRQ4_CCA_ED_DONE | IRQ5_AMI | IRQ6_TRX_UR)


/*---------------------------------------------------------------------------*/
/* Private functions */
static void radio_reset(void);
static void getLongAddr(uint8_t *addr, uint8_t len);;
static void setLongAddr(const uint8_t * addr, uint8_t len);
static void setShortAddr(uint16_t addr);
static uint16_t getShortAddr(void);
static uint16_t getPanID(void);
static void setPanID(uint16_t pan);
static void start_CTTM(uint8_t channel);
static void stop_CTTM(void);
/*---------------------------------------------------------------------------*/
/* HAL - hardware abstraction layer */
static uint8_t regRead(uint8_t addr);
static void regWrite(uint8_t addr, uint8_t value);
static uint8_t bitRead(uint8_t addr, uint8_t mask, uint8_t offset);
static void bitWrite(uint8_t addr, uint8_t mask, uint8_t offset, uint8_t value);
static int frameRead(rxFrame_t *frame);
static int frameWrite(txFrame_t *frame);
/*---------------------------------------------------------------------------*/

static rxFrame_t rxFrame;
static txFrame_t txFrame;

volatile static at86rf2xx_flags_t flags;
uint8_t packet_is_pending = 0;

/*---------------------------------------------------------------------------*/
int
at86rf2xx_init(void)
{
    LOG_DBG("%s\n", __func__);

    // Initialize I/O
    at86rf2xx_arch_init();

    // Reset internal and hardware states
	radio_reset();

    // Reset driver statistic
    AT86RF2XX_STATS_RESET();

#if AT86RF2XX_PACKET_STATS
    // Init buffers for packet statistics
    STATS_initBuff();
#endif
	// Start Contiki process which will take care of received packets
	process_start(&at86rf2xx_process, NULL);
	// process_start(&at86rf2xx_calibration_process, NULL);

    at86rf2xx_on();

	return 1;
}
/*---------------------------------------------------------------------------*/
int
at86rf2xx_prepare(const void *payload, unsigned short payload_len)
{
    AT86RF2XX_STATS_ADD(txTry);

    if (payload_len > AT86RF2XX_MAX_PAYLOAD_SIZE) {
        LOG_ERR("Payload larger than radio buffer: %u > %u\n", payload_len, AT86RF2XX_MAX_PAYLOAD_SIZE);
        AT86RF2XX_STATS_ADD(txError);

        return RADIO_TX_ERR;
    }

    memcpy(txFrame.content, payload, payload_len);
    txFrame.len = payload_len;

    LOG_DBG("Prepared %u bytes\n", payload_len);

#if !AT86RF2XX_CHECKSUM
    txFrame.crc = (uint16_t *)(txFrame.content + txFrame.len);
    *txFrame.crc = crc16_data(txFrame.content, txFrame.len, 0x00);
    // LOG_DBG("calculated CRC 0x%04x \n", *txFrame.crc);
#endif

    return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
int
at86rf2xx_transmit(unsigned short transmit_len)
{
    LOG_DBG("%s\n", __func__);

    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_PLL_ON);
    while(bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION){}
    flags.value = 0;

    at86rf2xx_arch_set_SLPTR();
    at86rf2xx_arch_clear_SLPTR();

    uint8_t status = frameWrite(&txFrame);
    if (status != 0){
        AT86RF2XX_STATS_ADD(txError);
        return RADIO_TX_ERR;
    }

    // Wait to complete BUSY STATE
    // BUSYWAIT_UNTIL(flags.TRX_END);   --> Vesna was in loop because no TRX_END flag appeared
    while(!flags.TRX_END || !flags.TRX_UR ){

        uint8_t trxStatus = bitRead(SR_TRX_STATUS);

        if(trxStatus == TRX_STATUS_TX_ON  || trxStatus == TRX_STATUS_TX_ARET_ON ){
            LOG_DBG("Other way \n");
            break;
        }
    }
    if(flags.TRX_UR){
        LOG_DBG("TRX_UR flag! \n");
        return RADIO_TX_ERR;
    }

    flags.value = 0;

    #if AT86RF2XX_PACKET_STATS
        // Update TX packet statistics
        STATS_txPush(&txFrame);
    #endif

    txFrame.trac = (AT86RF2XX_ARET) ? bitRead(SR_TRAC_STATUS) : TRAC_SUCCESS;
    
    ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);



    // Go to RX (Contiki does not distinguish between RX ON and TX_ON)
    // After TX, we should listen for ACK packet
    regWrite(RG_TRX_STATE, TRX_CMD_RX_ON);
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);


	switch (txFrame.trac) {
        case TRAC_SUCCESS:
            AT86RF2XX_STATS_ADD(txSuccess);
			LOG_DBG("TRAC=OK\n");
            return RADIO_TX_OK;

        case TRAC_NO_ACK:
            AT86RF2XX_STATS_ADD(txNoAck);
            LOG_DBG("TRAC=NO-ACK\n");
            return RADIO_TX_NOACK;

        case TRAC_CHANNEL_ACCESS_FAILURE:
            AT86RF2XX_STATS_ADD(txCollision);
            LOG_DBG("TRAC=collision\n");
            return RADIO_TX_COLLISION;

        default:
            AT86RF2XX_STATS_ADD(txError);
            LOG_DBG("TRAC=invalid (%u)\n", txFrame.trac);
            return RADIO_TX_ERR;
	}
}
/*---------------------------------------------------------------------------*/
int
at86rf2xx_send(const void *payload, unsigned short payload_len)
{
	at86rf2xx_prepare(payload, payload_len);
	return at86rf2xx_transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
int at86rf2xx_read(void *buf, unsigned short buf_len)
{
    int_master_status_t status;

    frameRead(&rxFrame);
    packet_is_pending = 0;

    #if AT86RF2XX_PACKET_STATS
        // Update RX packet statistics
        STATS_rxPush(&rxFrame);
    #endif

    uint8_t frame_len = rxFrame.len;
    status = critical_enter();

    memcpy(buf, rxFrame.content, rxFrame.len);
    rxFrame.len = 0;

    critical_exit(status);

    LOG_DBG("Got %u bytes\n", frame_len);
    return frame_len;
}
/*---------------------------------------------------------------------------*/
int
at86rf2xx_channel_clear(void)
{
    #if AT86RF2XX_HW_CCA
        // The radio will take care of it.
        return 1;
    #else
        uint8_t cca;
        //at86rf2xx_on();   // Contiki puts the radio to on state
        bitWrite(SR_RX_PDT_DIS, 1); // disable reception
        bitWrite(SR_CCA_REQUEST, 1); // trigger CCA sensing
        BUSYWAIT_UNTIL(flags.CCA);
        flags.CCA = 0;
        cca = bitRead(SR_CCA_STATUS); // 1 = IDLE, 0 = BUSY
        bitWrite(SR_RX_PDT_DIS, 0); // Enable reception
        return cca;
    #endif
}
/*---------------------------------------------------------------------------*/
int // Check if the radio driver is currently receiving a packet 
at86rf2xx_receiving_packet(void)
{
    if (flags.RX_START) {
        uint8_t trxState = bitRead(SR_TRX_STATUS);
        switch (trxState) {
            case TRX_STATUS_BUSY_RX:
            case TRX_STATUS_BUSY_RX_AACK:
            case TRX_STATUS_BUSY_RX_AACK_NOCLK:
                // in any busy Rx state
                return 1;

            default:
                // false alarm or already received a packet
                flags.RX_START = 0; 
                return 0;
        }
    }

    return 0;
}
/*---------------------------------------------------------------------------*/
int
at86rf2xx_pending_packet(void)
{
    #if !AT86RF2XX_CHECKSUM
    if(rxFrame.len > 0){
        uint16_t crc = crc16_data(rxFrame.content, rxFrame.len, 0x00);

        if(*rxFrame.crc != crc){
            //LOG_DBG("CRC missmatch: 0x%04x != 0x%04x \n", crc, *rxFrame.crc);
            rxFrame.len = 0;    // TODO is it ok?
            return 0;
        }
    }
    #endif 

    return packet_is_pending;
}
/*---------------------------------------------------------------------------*/
int
at86rf2xx_on(void)
{
    uint8_t trxState;

#if AT86RF2XX_AACK
    again:
        trxState = bitRead(SR_TRX_STATUS);
        switch (trxState) {
            case TRX_STATUS_STATE_TRANSITION:
                goto again;

            case TRX_STATUS_BUSY_RX:
            case TRX_STATUS_BUSY_TX:
            case TRX_STATUS_BUSY_TX_ARET:
                LOG_WARN("ON-Interrupted busy state %d\n", trxState);

            case TRX_STATUS_TX_ARET_ON:
            case TRX_STATUS_RX_ON:

                // First to TRX_OFF state
                regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);
                if (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION) {
                    goto again;
                }

            case TRX_STATUS_TRX_OFF:
            case TRX_STATUS_TX_ON:
            
                // Then go to RX_AACK_ON state
                regWrite(RG_TRX_STATE, TRX_CMD_RX_AACK_ON );
                if (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION) {
                    goto again;
                }

                ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);

            case TRX_STATUS_RX_AACK_ON:

                // Allready in proper state
                flags.value = 0;
                ENERGEST_ON(ENERGEST_TYPE_LISTEN);
                return 1;

            case TRX_STATUS_BUSY_RX_AACK:
                LOG_WARN("Allready receiving something \n");
                return 1;

            default:
                LOG_ERR("ON-Unknown state: 0x%02x\n", trxState);
                // Contiki doesn't care if we return back 1 or 0...we should reset radio at this point
                
                regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);
                while (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION); 
                regWrite(RG_TRX_STATE, TRX_CMD_RX_AACK_ON);
                while (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION);
                flags.value = 0;
                ENERGEST_ON(ENERGEST_TYPE_LISTEN);
                return 1;
        }
#else
    again:
        trxState = bitRead(SR_TRX_STATUS);
        switch (trxState) {
            case TRX_STATUS_STATE_TRANSITION:
                goto again;

            case TRX_STATUS_BUSY_RX_AACK:
            case TRX_STATUS_BUSY_TX:
            case TRX_STATUS_BUSY_TX_ARET:
                LOG_WARN("ON-Interrupted busy state %d!\n", trxState);

            case TRX_STATUS_TX_ARET_ON:
            case TRX_STATUS_RX_AACK_ON:
            case TRX_STATUS_P_ON:
                // First go to TRX_OFF state
                regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);
                if (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION) {
                    goto again;
                }

            case TRX_STATUS_TX_ON:
            case TRX_STATUS_TRX_OFF:
                // Then go to RX_ON state
                regWrite(RG_TRX_STATE, TRX_CMD_RX_ON );
                if (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION) {
                    goto again;
                }
            
            case TRX_STATUS_RX_ON:
                // Allready in proper state
                flags.value = 0;
                ENERGEST_ON(ENERGEST_TYPE_LISTEN);
                return 1;

            case TRX_STATUS_BUSY_RX:
                LOG_DBG("Allready receiving something \n");
                return 1;

            default:
                LOG_ERR("ON-Unknown state: 0x%02x\n", trxState);
                // Contiki doesn't care if we return back 1 or 0...we should reset radio at this point
                goto again;

                //regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);
                //while (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION); 
                //regWrite(RG_TRX_STATE, TRX_CMD_RX_ON );
                //while (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION); 
                //flags.value = 0;
                //ENERGEST_ON(ENERGEST_TYPE_LISTEN);
                //return 1;
        }

#endif
}
/*---------------------------------------------------------------------------*/
int
at86rf2xx_off(void)
{
    uint8_t trxState;

again:
    trxState = bitRead(SR_TRX_STATUS);
    switch (trxState) {
        case TRX_STATUS_STATE_TRANSITION:
            goto again;

        case TRX_STATUS_TRX_OFF:
            // already in OFF state
            return 1;

        case TRX_STATUS_RX_ON:
        case TRX_STATUS_RX_AACK_ON:  
        case TRX_STATUS_TX_ON:
        case TRX_STATUS_TX_ARET_ON:
        case TRX_STATUS_P_ON:

            // Idle Tx/Rx state
            regWrite(RG_TRX_STATE, TRX_CMD_TRX_OFF);
            if (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION) {
                goto again;
            }
            flags.value = 0;
            ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
            return 1;

        case TRX_STATUS_RX_AACK_ON_NOCLK:
        case TRX_STATUS_BUSY_RX:
        case TRX_STATUS_BUSY_RX_AACK:
        case TRX_STATUS_BUSY_RX_AACK_NOCLK:
        case TRX_STATUS_BUSY_TX:
        case TRX_STATUS_BUSY_TX_ARET:

            // Busy states
            LOG_WARN("OFF-Interrupted busy state %d\n", trxState);
            regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);
            if (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION) {
                goto again;
            }
            flags.value = 0;
            ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
            
            return 0;

        default:
            LOG_ERR("OFF-Unknown state: 0x%02x\n", trxState);
            goto again;

            // Contiki doesn't care if we return back 1 or 0...we should reset radio at this point
            //regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);
            //while (bitRead(SR_TRX_STATUS) == TRX_STATUS_STATE_TRANSITION); 
            //flags.value = 0;
            //ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
            //return 1;
    }
}
/*---------------------------------------------------------------------------*/
void
at86rf2xx_isr(void)
{
    ENERGEST_ON(ENERGEST_TYPE_IRQ);

    at86rf2xx_irq_t irq;
    irq.value = regRead(RG_IRQ_STATUS);

    if (irq.IRQ1_PLL_UNLOCK) {
        flags.PLL_LOCK = 0;
    }

    if (irq.IRQ0_PLL_LOCK) {
        flags.PLL_LOCK = 1;
    }

    if (irq.IRQ2_RX_START) {
        flags.RX_START = 1;
        flags.AMI = 0;
        flags.TRX_END = 0;

        rxFrame.timestamp = RTIMER_NOW();
        AT86RF2XX_STATS_ADD(rxDetected);
    }

    if (irq.IRQ5_AMI) {
        flags.AMI = 1;
        AT86RF2XX_STATS_ADD(rxAddrMatch);
    }

    if (irq.IRQ6_TRX_UR) {
        flags.TRX_UR = 1;
    }

    if (irq.IRQ3_TRX_END) {
        flags.TRX_END = 1;

        if (flags.RX_START) {

            flags.RX_START = 0;
            packet_is_pending = 1;

            process_poll(&at86rf2xx_process);
 
            AT86RF2XX_STATS_ADD(rxSuccess);
        } else {
            AT86RF2XX_STATS_ADD(txCount);
        }
    }

	if (irq.IRQ4_AWAKE_END) { // CCA_***_IRQ
		flags.SLEEP = 0;
		flags.CCA = 1;

	}

	if (irq.IRQ7_BAT_LOW) {}

    ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(at86rf2xx_process, ev, data)
{
	int len;
	PROCESS_BEGIN();

	LOG_INFO("AT86RF2xx driver process started!\n");

	while(1) {
		PROCESS_YIELD_UNTIL(!AT86RF2XX_POLLING_MODE && ev == PROCESS_EVENT_POLL);
        AT86RF2XX_STATS_ADD(rxToStack);

        packetbuf_clear();
        // Depricated in Contiki-NG v4.9
        //packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, rxFrame.timestamp);
        len = at86rf2xx_read(packetbuf_dataptr(), PACKETBUF_SIZE);

        if(len) {
            packetbuf_set_datalen(len);
            NETSTACK_MAC.input();
        }
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
	if (!value) return RADIO_RESULT_INVALID_VALUE;

	switch (param) {
		case RADIO_PARAM_POWER_MODE:
        {
            uint8_t trxState = bitRead(SR_TRX_STATUS);
            switch (trxState) {
                case TRX_STATUS_RX_ON:
                case TRX_STATUS_RX_ON_NOCLK:
                case TRX_STATUS_RX_AACK_ON:
                case TRX_STATUS_RX_AACK_ON_NOCLK:
                case TRX_STATUS_BUSY_RX:
                case TRX_STATUS_BUSY_RX_AACK:
                case TRX_STATUS_BUSY_RX_AACK_NOCLK:
                    *value = RADIO_POWER_MODE_ON;
                    return RADIO_RESULT_OK;

                case TRX_STATUS_TX_ON:
                case TRX_STATUS_TX_ARET_ON:
                case TRX_STATUS_BUSY_TX:
                case TRX_STATUS_BUSY_TX_ARET:
                    *value = RADIO_POWER_MODE_CARRIER_ON;
                    return RADIO_RESULT_OK;

                case TRX_STATUS_TRX_OFF:
                default:
                    *value = RADIO_POWER_MODE_OFF;
                    return RADIO_RESULT_OK;
            }
        }

		case RADIO_PARAM_CHANNEL:
			*value = (radio_value_t)bitRead(SR_CHANNEL);
			return RADIO_RESULT_OK;

		case RADIO_PARAM_PAN_ID:
			*value = (radio_value_t)getPanID();
			return RADIO_RESULT_OK;

		case RADIO_PARAM_16BIT_ADDR:
			*value = (radio_value_t)getShortAddr();
			return RADIO_RESULT_OK;

		case RADIO_PARAM_RX_MODE:
            *value = 0;
			if (AT86RF2XX_AACK && !AT86RF2XX_PROMISCOUS_MODE) *value |= RADIO_RX_MODE_ADDRESS_FILTER;
			if (AT86RF2XX_AACK) *value |= RADIO_RX_MODE_AUTOACK;
			if (AT86RF2XX_POLLING_MODE) *value |= RADIO_RX_MODE_POLL_MODE;
            return RADIO_RESULT_OK;

		case RADIO_PARAM_TX_MODE:
            *value = 0;
			if (!AT86RF2XX_HW_CCA) *value |= RADIO_TX_MODE_SEND_ON_CCA;

			return RADIO_RESULT_OK;

		case RADIO_PARAM_TXPOWER:
			*value = (radio_value_t)bitRead(SR_TX_PWR);
			return RADIO_RESULT_OK;

		case RADIO_PARAM_CCA_THRESHOLD:
			return AT86RSSI_BASE_VAL + 2 * (radio_value_t)bitRead(SR_CCA_ED_THRES);

		case RADIO_PARAM_RSSI:
			*value = (3 * ((radio_value_t)bitRead(SR_RSSI) - 1) + AT86RSSI_BASE_VAL);
			return RADIO_RESULT_OK;

    case RADIO_PARAM_LAST_RSSI:
        *value = (radio_value_t)rxFrame.rssi;
        return RADIO_RESULT_OK;

		case RADIO_PARAM_LAST_LINK_QUALITY:
			*value = (radio_value_t)rxFrame.lqi;
			return RADIO_RESULT_OK;

		case RADIO_CONST_CHANNEL_MIN:
			*value = 11;
			return RADIO_RESULT_OK;

		case RADIO_CONST_CHANNEL_MAX:
			*value = 26;
			return RADIO_RESULT_OK;

		case RADIO_CONST_TXPOWER_MIN:
			*value = 0xF;
			return RADIO_RESULT_OK;

		case RADIO_CONST_TXPOWER_MAX:
			*value = 0x0;
			return RADIO_RESULT_OK;

		case RADIO_CONST_PHY_OVERHEAD:
			*value = (radio_value_t)AT86RF2XX_PHY_OVERHEAD;
			return RADIO_RESULT_OK;

		case RADIO_CONST_BYTE_AIR_TIME:
			*value = (radio_value_t)AT86RF2XX_BYTE_AIR_TIME;
			return RADIO_RESULT_OK;

		case RADIO_CONST_DELAY_BEFORE_TX:
			*value = (radio_value_t)AT86RF2XX_DELAY_BEFORE_TX;
			return RADIO_RESULT_OK;

		case RADIO_CONST_DELAY_BEFORE_RX:
			*value = (radio_value_t)AT86RF2XX_DELAY_BEFORE_RX;
			return RADIO_RESULT_OK;

		case RADIO_CONST_DELAY_BEFORE_DETECT:
			*value = (radio_value_t)AT86RF2XX_DELAY_BEFORE_DETECT;
			return RADIO_RESULT_OK;


    case RADIO_CONST_MAX_PAYLOAD_LEN:
        *value = (radio_value_t)AT86RF2XX_MAX_PAYLOAD_SIZE;
        return RADIO_RESULT_OK;

		default:
			return RADIO_RESULT_NOT_SUPPORTED;
	}

}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
	switch (param) {
        case RADIO_PARAM_POWER_MODE:
            switch (value) {
                case RADIO_POWER_MODE_ON:
                    at86rf2xx_on();
                    return RADIO_RESULT_OK;

                case RADIO_POWER_MODE_OFF:
                    at86rf2xx_off();
                    return RADIO_RESULT_OK;

                default:
                    return RADIO_RESULT_INVALID_VALUE;
            }

        case RADIO_PARAM_CHANNEL:
            if (value < 11 || value > 26) {
                return RADIO_RESULT_INVALID_VALUE;
            }
            bitWrite(SR_CHANNEL, value);
            return RADIO_RESULT_OK;

        case RADIO_PARAM_PAN_ID:
            setPanID(value);
            return RADIO_RESULT_OK;

        case RADIO_PARAM_16BIT_ADDR:
            setShortAddr(value);
            return RADIO_RESULT_OK;

        case RADIO_PARAM_RX_MODE:
            //AT86RF2XX_PROMISCOUS_MODE = (value & RADIO_RX_MODE_ADDRESS_FILTER) > 0;

            // Configure Promiscuous mode (AACK-mode only)
            //bitWrite(SR_AACK_PROM_MODE, AT86RF2XX_PROMISCOUS_MODE);
            //bitWrite(SR_AACK_UPLD_RES_FT, AT86RF2XX_PROMISCOUS_MODE);
            //bitWrite(SR_AACK_FLTR_RES_FT, AT86RF2XX_PROMISCOUS_MODE);

            //AT86RF2XX_AACK = (value & RADIO_RX_MODE_AUTOACK) > 0;
            //AT86RF2XX_POLLING_MODE = (value & RADIO_RX_MODE_POLL_MODE) > 0;


        {
            bool addrFilter = (value & RADIO_RX_MODE_ADDRESS_FILTER) > 0;
            bool aackMode = (value & RADIO_RX_MODE_AUTOACK) > 0;
            bool pollMode = (value & RADIO_RX_MODE_POLL_MODE) > 0;

            if (addrFilter != (AT86RF2XX_AACK && !AT86RF2XX_PROMISCOUS_MODE)) {
                //LOG_ERR("Invalid ADDR_FILTER settings\n");
            }

            if (aackMode != AT86RF2XX_AACK) {
                LOG_ERR("Invalid AT86RF2XX_AACK settings\n");
            }

            if (pollMode != AT86RF2XX_POLLING_MODE) {
                LOG_ERR("Invalid AT86RF2XX_POLLING_MODE settings\n");
            }

        }
            return RADIO_RESULT_OK;
            //return RADIO_RESULT_NOT_SUPPORTED;
    
        case RADIO_PARAM_TX_MODE:
        {
            // TODO: Make it shorter
            bool sendOnCCA = (value & RADIO_TX_MODE_SEND_ON_CCA) > 0;
            if (sendOnCCA != AT86RF2XX_HW_CCA) { // They are mutually exclusive
                LOG_ERR("Invalid AT86RF2XX_HW_CCA settings\n");
                return RADIO_RESULT_ERROR;
            }

            return RADIO_RESULT_OK;
        }
        case RADIO_PARAM_TXPOWER:
            if (value < 0 || value > 0xF) {
                return RADIO_RESULT_INVALID_VALUE;
            }
            bitWrite(SR_TX_PWR, value);
            return RADIO_RESULT_OK;

        case RADIO_PARAM_CCA_THRESHOLD:
            bitWrite(SR_CCA_ED_THRES, value / 2 + 91);
            return RADIO_RESULT_OK;

        case RADIO_PARAM_SHR_SEARCH:
        default:
            return RADIO_RESULT_NOT_SUPPORTED;
	}
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
	if (dest == NULL) return RADIO_RESULT_ERROR;

	switch (param) {
		case RADIO_PARAM_64BIT_ADDR:
			getLongAddr((uint8_t *)dest, size);
			return RADIO_RESULT_OK;

		case RADIO_PARAM_LAST_PACKET_TIMESTAMP:
		    *(rtimer_clock_t *)dest = rxFrame.timestamp;
		    return RADIO_RESULT_OK;

        #if MAC_CONF_WITH_TSCH
		case RADIO_CONST_TSCH_TIMING:
            LOG_INFO("Reading radio's RADIO_CONST_TSCH_TIMING matrix\n");
			*(const uint16_t **)dest = AT86RF2XX_DEFAULT_TIMESLOT_TIMING;
        #endif

		default:
			return RADIO_RESULT_NOT_SUPPORTED;
	}
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
	if (src == NULL) return RADIO_RESULT_ERROR;

	switch (param) {
		case RADIO_PARAM_64BIT_ADDR:
			setLongAddr((const uint8_t *)src, size);
			return RADIO_RESULT_OK;

		default:
			return RADIO_RESULT_NOT_SUPPORTED;
	}
}
/*---------------------------------------------------------------------------*/
const struct radio_driver at86rf2xx_driver = {
	.init = at86rf2xx_init,
	.prepare = at86rf2xx_prepare,
	.transmit = at86rf2xx_transmit,
	.send = at86rf2xx_send,
	.read = at86rf2xx_read,
	.channel_clear = at86rf2xx_channel_clear,
	.receiving_packet = at86rf2xx_receiving_packet,
	.pending_packet = at86rf2xx_pending_packet,
	.on = at86rf2xx_on,
	.off = at86rf2xx_off,
	.get_value = get_value,
	.set_value = set_value,
	.get_object = get_object,
	.set_object = set_object,
};




/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Private functions */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static void
setPanID(uint16_t pan)
{
	regWrite(RG_PAN_ID_0, pan & 0xFF);
	regWrite(RG_PAN_ID_1, pan >> 8);
	LOG_DBG("PAN == 0x%02x\n", pan);
}
/*---------------------------------------------------------------------------*/
static uint16_t
getPanID(void)
{
	uint16_t pan = ((uint16_t)regRead(RG_PAN_ID_1) << 8) & 0xFF00;
    pan |= (uint16_t)regRead(RG_PAN_ID_0) & 0xFF;
    return pan;
}
/*---------------------------------------------------------------------------*/
static uint16_t
getShortAddr(void)
{
	uint16_t addr = ((uint16_t)regRead(RG_SHORT_ADDR_1) << 8) & 0xFF00;
    addr |= (uint16_t)regRead(RG_SHORT_ADDR_0) & 0xFF;
    return addr;
}
/*---------------------------------------------------------------------------*/
static void
setShortAddr(uint16_t addr)
{
	regWrite(RG_SHORT_ADDR_0, addr & 0xFF);
	regWrite(RG_SHORT_ADDR_1, addr >> 8);
	LOG_DBG("Short addr == 0x%02x\n", addr);
}
/*---------------------------------------------------------------------------*/
static void
setLongAddr(const uint8_t * addr, uint8_t len)
{
    if (len > 8) len = 8;

    // The usual representation of MAC address has big-endianess. However,
    // This radio uses little-endian, so the order of bytes has to be reversed.
	// When we define IEEE addr, the most important byte is ext_addr[0], while
	// on radio RG_IEEE_ADDR_0 must contain the lowest/least important byte.
    for (uint8_t i = 0; i < len; i++) {
        regWrite(RG_IEEE_ADDR_7 - i, addr[i]);
    }
}
/*---------------------------------------------------------------------------*/
static void
getLongAddr(uint8_t *addr, uint8_t len)
{
	if (len > 8) len = 8;
    for (uint8_t i = 0; i < len; i++) {
        addr[i] = regRead(RG_IEEE_ADDR_7 - i);
    }
}
/*---------------------------------------------------------------------------*/
static void
radio_reset(void)
{
    uint8_t dummy __attribute__((unused));

    LOG_DBG("%s\n", __func__);

    at86rf2xx_arch_disable_EXTI();  // disable interrupts
	at86rf2xx_arch_set_RSTN();      // reset the radio
    at86rf2xx_arch_clear_SLPTR();   // prevent going to sleep
    at86rf2xx_arch_clear_EXTI();    // clear interrupt flag
    at86rf2xx_arch_clear_RSTN();    // release radio from RESET state

	
    uint8_t at86rf2xxChip = AT86RF2XX_ID_UNDEFINED;

    /* Radio indentification procedure */
    do {
        LOG_DBG("Detecting AT86RF2xx radio\n");

        // Match JEDEC manufacturer ID
        if (regRead(RG_MAN_ID_0) == AT86RF2XX_MAN_ID_0 && regRead(RG_MAN_ID_1) == AT86RF2XX_MAN_ID_1){
            LOG_DBG("JEDEC ID matches Atmel\n");

            // Match known radio (and sanitize radio type)
            at86rf2xxChip = regRead(RG_PART_NUM);
        }
    } while (AT86RF2XX_ID_UNDEFINED == at86rf2xxChip);

    if (AT86RF2XX_ID != at86rf2xxChip){
        LOG_ERR("Defined RADIO is not the same as the one on the board!\n");
        printf("\n");
        while(1);
    }

    // Radio identified. Now, put it into TRX_OFF state.
    do {
        regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);
    } while (TRX_STATUS_TRX_OFF != bitRead(SR_TRX_STATUS));

	// CLKM clock change visible (0 = immediately, 1 = needs to go through SLEEP cycle)
	bitWrite(SR_CLKM_SHA_SEL, 0);

#if AT86RF2XX_ENABLE_CLKM
	// Enable CLKM (as output) so it can be used as external clock source for VESNA
	bitWrite(SR_CLKM_CTRL, CLKM_CTRL__8MHz);
#else
    // Disable CLKM since it's not connected anywhere (ISMTV)
    bitWrite(SR_CLKM_CTRL, CLKM_CTRL__DISABLED);
#endif

	// Enable/disable Tx autogenerating CRC16/CCITT
	bitWrite(SR_TX_AUTO_CRC_ON, AT86RF2XX_CHECKSUM);

	// Enable RX_SAFE mode to protect buffer while reading it
	bitWrite(SR_RX_SAFE_MODE, 1);

	// Set same value for RF231 (default=0) and RF233 (default=1)
	bitWrite(SR_IRQ_MASK_MODE, 1);

	// Number of CSMA retries (part of IEEE 802.15.4)
	// Possible values [0 - 5], 6 is reserved, 7 will send immediately (no CCA)
	bitWrite(SR_MAX_CSMA_RETRIES, AT86RF2XX_HW_CCA ? AT86RF2XX_CSMA_RETRIES : 7);

	// Number of maximum TX_ARET frame retries
	// Possible values [0 - 15]
	bitWrite(SR_MAX_FRAME_RETRIES, AT86RF2XX_FRAME_RETRIES);

	// Highest allowed backoff exponent
	regWrite(RG_CSMA_BE, 0x80);

    // Randomize backoff timing
	// Upper two RSSI reg bits are random
	regWrite(RG_CSMA_SEED_0, regRead(RG_PHY_RSSI));

	// First returned byte will be IRQ_STATUS;
	// bitWrite(SR_SPI_CMD_MODE, SPI_CMD_MODE__IRQ_STATUS); // Disable, since we don't need it (TODO: true for RF233, check for others)

	// Configure Promiscuous mode (AACK-mode only)
	bitWrite(SR_AACK_PROM_MODE, AT86RF2XX_PROMISCOUS_MODE);
	bitWrite(SR_AACK_UPLD_RES_FT, AT86RF2XX_PROMISCOUS_MODE);
	bitWrite(SR_AACK_FLTR_RES_FT, AT86RF2XX_PROMISCOUS_MODE);

    // Allow TSCH packet through the frame filter (TSHC is 2015 --> version 2)
    // TODO: check if needed (CSMA works with this..)
    bitWrite(SR_AACK_FVN_MODE, 2);

	// Enable only specific IRQs
	regWrite(RG_IRQ_MASK, DEFAULT_IRQ_MASK);

	// Read IRQ register to clear it
	dummy = regRead(RG_IRQ_STATUS);

	// Clear any interrupt pending
	at86rf2xx_arch_clear_EXTI();
    at86rf2xx_arch_enable_EXTI();
}

/*---------------------------------------------------------------------------*/
static void
start_CTTM(uint8_t channel)
{
    static txFrame_t continuousFrame;
    uint8_t status;
    uint8_t payload[127];
    uint8_t payload_len = 127;

// 1. Reset AT86RF212 radio 
    at86rf2xx_arch_set_RSTN();       // Hold radio in reset state 
	at86rf2xx_arch_clear_EXTI();    // Clear interrupt flag
    flags.value = 0;
	at86rf2xx_arch_spi_deselect();      // clear chip select (default)
	at86rf2xx_arch_clear_SLPTR();   // prevent going to sleep
    at86rf2xx_arch_clear_RSTN();     // Release radio from RESET state

// 2. Enable PLL_LOCK IRQ 
    regWrite(RG_IRQ_MASK, IRQ0_PLL_LOCK);

// 3. Disable TX_AUTO_CRC_ON
    bitWrite(SR_TX_AUTO_CRC_ON, 0);

// 4. Set radio to TRX_OFF state 
    regWrite(RG_TRX_STATE, TRX_CMD_TRX_OFF);

// 5. Set clock at pin 17 (CLKM) 

// 6. Set channel 
    bitWrite(SR_CHANNEL, channel);

// 7. Set output power to (max = 0x0)
    bitWrite(SR_TX_PWR, 0x0);

// 8. Verify TRX_OFF state 
    while(bitRead(SR_TRX_STATUS) != TRX_STATUS_TRX_OFF){
        LOG_WARN("CTTM state error \n");
        regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);
    }

// 9. Enable Continuous transmission Test mode - step #1 
    regWrite(0x36, 0x0F);

// #if CW
// 10. Enable High Data Rate Mode, 2 Mb/s 
    regWrite(0x0C, 0x03);

// 11. Configure High Data rate Mode 
#ifdef AT86RF233
    regWrite(0x0A, 0x37);
#else
    regWrite(0x0A, 0xA7);
#endif

// 12. Write PHR and PSDU data - frame buffer - choose one
    memset(payload, 0x00, payload_len);   // CW at Fc-0.5MHz
    //memset(payload, 0xFF, payload_len);   // CW at Fc+0.5MHz

// #endif CW


/* #if PRBS
// 10. 11. and 12. 
    memset(payload, 0xBB, payload_len);
// #endif    */

    memcpy(continuousFrame.content, payload, payload_len);
    continuousFrame.len = payload_len;

    status = frameWrite(&continuousFrame);
    if (status != 0){
        LOG_ERR("SPI frame buffer write error\n ");
    }

// 13. Enable Continuous transmission Test mode - step #2
    regWrite(0x1C, 0x54);

// 14. Enable Continuous transmission Test mode - step #3 
    regWrite(0x1C, 0x46);

// 15. Go to PLL_ON state 
    regWrite(RG_TRX_STATE, TRX_CMD_PLL_ON);

// 16. Wait for IRQ PLL_LOCK 
    while(!flags.PLL_LOCK); 
    flags.value = 0;

// 17. Initiate transmission (issue TX_START command) 
    regWrite(RG_TRX_STATE, TRX_CMD_TX_START);

// 18. Preform measurement 
    LOG_INFO("Radio is continuously transmitting on channel %d \n", channel);
}
/*---------------------------------------------------------------------------*/
static void
stop_CTTM(void){

// 19. Disable continuous transmission test mode 
    regWrite(0x1C, 0x0);

// 20. Reset radio 
    radio_reset();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* HAL - hardware abstraction layer */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

inline static uint8_t
REGREAD(uint8_t addr, uint8_t *value)
{
    uint8_t status = 0;
    int_master_status_t intStatus;

    intStatus = critical_enter();
    at86rf2xx_arch_spi_select();
    status = at86rf2xx_arch_spi_txrx(((addr & CMD_REG_MASK) | CMD_REG_ACCESS | CMD_READ), value);
    status |= at86rf2xx_arch_spi_txrx(0x00, value);
    at86rf2xx_arch_spi_deselect();
    critical_exit(intStatus);

    if (status != 0) LOG_WARN("Reg read error (0x%02x)\n", addr);
    return status;
}
/*---------------------------------------------------------------------------*/
inline static uint8_t
REGWRITE(uint8_t addr, const uint8_t value)
{
    uint8_t status = 0;
    int_master_status_t intStatus;
    uint8_t dummy __attribute__((unused));

    intStatus = critical_enter();
    at86rf2xx_arch_spi_select();
    status = at86rf2xx_arch_spi_txrx(((addr & CMD_REG_MASK) | CMD_REG_ACCESS | CMD_WRITE), &dummy);
    status |= at86rf2xx_arch_spi_txrx(value, &dummy);
    at86rf2xx_arch_spi_deselect();
    critical_exit(intStatus);

    if (status != 0) LOG_WARN("Reg write error (0x%02x)\n", addr);
    return status;
}
/*---------------------------------------------------------------------------*/
inline static uint8_t
FIFOREAD(rxFrame_t *frame)
{
    uint8_t status;
    int_master_status_t intStatus;
    at86rf2xx_irq_t irq;
    uint8_t dummy __attribute__((unused));

    intStatus = critical_enter();

    at86rf2xx_arch_spi_select();
    status = at86rf2xx_arch_spi_txrx((CMD_FB_ACCESS | CMD_READ), &irq.value);
    //if (irq.IRQ6_TRX_UR) goto error;

    status |= at86rf2xx_arch_spi_txrx(0x00, &frame->len);

    if (frame->len < 3 || frame->len > AT86RF2XX_MAX_FRAME_SIZE) {
        goto error;
    }
    frame->len -= AT86RF2XX_CRC_SIZE;
    frame->crc = (uint16_t *)(frame->content + frame->len);

    /**
     * Note, that max frame length is limited to 255 with this implementation
     */
    for (uint8_t i = 0; i < frame->len + AT86RF2XX_CRC_SIZE; i++) {
        if (status != 0) goto error;
        status = at86rf2xx_arch_spi_txrx(0x00, frame->content + i);
    }

    status = at86rf2xx_arch_spi_txrx(0x00, &frame->lqi);

    /* AT86RF233 adds 2 more bytes, which are ED value and RX_STATUS */
#ifdef AT86RF233
    status |= at86rf2xx_arch_spi_txrx(0x00, (uint8_t *)&frame->ed);
    status |= at86rf2xx_arch_spi_txrx(0x00, (uint8_t *)&frame->rx_status);
#endif
    
    at86rf2xx_arch_spi_deselect();

    /* Obtain also the RSSI value, required by Contiki */
    status |= REGREAD(RG_PHY_ED_LEVEL, (uint8_t *)&frame->rssi);
    if (status != 0) goto error;

    critical_exit(intStatus);

    /* Radio's AT86RSSI_BASE_VAL differs from radio to radio - adjust accordingly */
    frame->rssi +=  AT86RSSI_BASE_VAL;

    return status;

error:
    at86rf2xx_arch_spi_deselect();
    frame->len = 0;
    critical_exit(intStatus);
    LOG_WARN("Frame read error\n");
    return status;
}
/*---------------------------------------------------------------------------*/
inline static uint8_t
FIFOWRITE(txFrame_t *frame)
{
    uint8_t status;
    int_master_status_t intStatus;
    uint8_t dummy __attribute__((unused));

    intStatus = critical_enter();

    at86rf2xx_arch_spi_select();

    status = at86rf2xx_arch_spi_txrx((CMD_FB_ACCESS | CMD_WRITE), &dummy);
    status |= at86rf2xx_arch_spi_txrx(frame->len + AT86RF2XX_CRC_SIZE, &dummy);

    /**
     * Note, that max frame length is limited to 255 with this implementation
     */
    for (uint8_t i = 0; i < frame->len + AT86RF2XX_CRC_SIZE; i++) {
        if (status != 0) goto error;
        status = at86rf2xx_arch_spi_txrx(frame->content[i], &dummy);
    }

    #if !AT86RF2XX_CHECKSUM
        for (uint8_t i = 0; i < AT86RF2XX_CRC_SIZE; i++) {
            status = at86rf2xx_arch_spi_txrx((uint8_t *)frame->crc+i, &dummy);
            if (status != 0) goto error;
        }
    #endif

error:
    at86rf2xx_arch_spi_deselect();
    critical_exit(intStatus);
    if (status != 0) LOG_WARN("Frame write error\n");
    return status;
}
/*---------------------------------------------------------------------------*/
static uint8_t
regRead(uint8_t addr)
{
    uint8_t value;
    while (0 != REGREAD(addr, &value));
    return value;
}
/*---------------------------------------------------------------------------*/
static void
regWrite(uint8_t addr, uint8_t value)
{
    while (0 != REGWRITE(addr, value));
}
/*---------------------------------------------------------------------------*/
static uint8_t
bitRead(uint8_t addr, uint8_t mask, uint8_t offset)
{
    uint8_t value;

    while(0 != REGREAD(addr, &value));
    value = (value & mask) >> offset;
    return value;
}
/*---------------------------------------------------------------------------*/
static void
bitWrite(uint8_t addr, uint8_t mask, uint8_t offset, uint8_t value)
{
    uint8_t tmp;

    while (0 != REGREAD(addr, &tmp));
    tmp = (tmp & ~mask) | ((value << offset) & mask);
    while (0 != REGWRITE(addr, tmp));
}
/*---------------------------------------------------------------------------*/
static int
frameRead(rxFrame_t *frame)
{
    uint8_t status = FIFOREAD(frame);
    return status;
}
/*---------------------------------------------------------------------------*/
static int
frameWrite(txFrame_t *frame)
{
    uint8_t status = FIFOWRITE(frame);
    return status;
}
