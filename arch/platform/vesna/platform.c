#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "vsn.h"
#include "vsncrc32.h"
#include "vsnpm.h"
#include "vsnsetup.h"
#include "vsnusart.h"
#include "vsntime.h"


#include "lib/crc16.h"

#include "contiki.h"
#include "dev/uart1.h"
#include "dev/serial-line.h"
#include "net/netstack.h"
#include "net/mac/framer/frame802154.h"
#include "net/linkaddr.h"
#include "lib/crc16.h"

#include "vesna-conf.h"
#include "board.h"

#include "sys/platform.h"
#include "sys/node-id.h"
/*---------------------------------------------------------------------------*/
#include "sys/log.h"
#define LOG_MODULE "VESNA"
#define LOG_LEVEL (LOG_LEVEL_MAIN)
/*---------------------------------------------------------------------------*/
typedef union stm32f1_uuid {
	uint32_t u32[3];
	uint16_t u16[6];
	uint8_t u8[12];
} stm32f1_uuid_t;

// STM32 has 96 bits "universal unique ID" on address 0x1FFFF7E8
#define STM32F1_UUID	(*(const stm32f1_uuid_t * const)0x1FFFF7E8)
/*---------------------------------------------------------------------------*/
/**
 * \brief Board specific iniatialisation
 */
void board_init(void);
/*---------------------------------------------------------------------------*/
void
platform_init_stage_one(void)
{
	// justinc, TOP_PAD is incorrect when first calling newlib malloc.
    mallopt(-2, 0); // #define M_TOP_PAD           -2

    // Reset clock configuration
    SystemInit();

    // Enable power management (enable/disable features/pins/hw)
    vsnPM_init();

    // VESNA uses internal clock generator, that's why is limited to 64MHz.
    // For up to 72MHz it would require external oscilator/clock generator.
    // VESNA SNC doesn't have own one. 64MHz is the max on HSI oscilator.

    vsnSetup_intClk(SNC_CLOCK_64MHZ);

    // - Enable GPIO clocks
    // - Enable interrupts
    // - Enable RTC, Tick,
    vsnSetup_initSnc();

    vsnSetup_calibHsi();

    vsnPM_mesureAdcBitVolt();
    vsnCRC32_init();

#if VESNA_UART1_ENABLED
	uart1_init(BAUD2UBR(VESNA_UART1_BAUDRATE));
	LOG_INFO("Hello there!\n");
#endif
}
/*---------------------------------------------------------------------------*/
void
platform_init_stage_two(void)
{

#if VESNA_SLIP_ENABLED
	slip_arch_init();
	LOG_DBG("UART1 as SLIP interface enabled\n");
#else
	uart1_set_input(serial_line_input_byte);
	process_start(&uart1_rx_process, NULL);
	// TODO: this process can be optimized
	serial_line_init();
	LOG_DBG("UART1 as serial input enabled\n");
#endif

	board_init();

	uint16_t uuid = crc16_data(STM32F1_UUID.u8, sizeof(STM32F1_UUID.u8), 0);

	// We define unique 64-bit address to represent this device.
	const uint8_t extAddr[8] = { 0, 0x12, 0x4B, 0, 0, 0x06, (uuid & 0xFF), (uuid >> 8) };

	// populate linkaddr_node_addr. Maintain endianess;
	// This sets MAC address
	memcpy(linkaddr_node_addr.u8, extAddr + 8 - LINKADDR_SIZE, LINKADDR_SIZE);
}
/*---------------------------------------------------------------------------*/
void
platform_init_stage_three(void)
{
	NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, IEEE802154_PANID);
	NETSTACK_RADIO.set_value(RADIO_PARAM_16BIT_ADDR, node_id);
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, IEEE802154_DEFAULT_CHANNEL);
	NETSTACK_RADIO.set_object(RADIO_PARAM_64BIT_ADDR, linkaddr_node_addr.u8, LINKADDR_SIZE);

	//process_start(&sensors_process, NULL);

#if VESNA_USE_EXTERNAL_CLOCK
	/* Go for external clock source. Do it only after the input clock is 
	 * properly configured - e.g. when AT86RF2XX radio is configured. */
	LOG_INFO("Switching to external clock source\n");
	vsnTime_delayS(1);
	int status = vsnSetup_extClk();
	if (status != SUCCESS) {
		vsnTime_delayS(1);
		LOG_WARN("No external clock source found!.. falling back to internal HSI 8MHz\n");
	}
#endif
}
/*---------------------------------------------------------------------------*/
void
platform_idle()
{
	// lpm_enter();
}
