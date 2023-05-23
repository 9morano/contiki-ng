/**
 * \file
 *     AT86RF215 OpenMoteB arch specific code
 * 
 * \author
 *      Grega Morano <grega.morano@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "dev/spi-arch-legacy.h"
#include "dev/spi-legacy.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/gpio-hal.h"
/*---------------------------------------------------------------------------*/
#define AT86RF215_SPI_CSN_PORT_BASE   GPIO_PORT_TO_BASE(AT86RF215_SPI_CSN_PORT)
#define AT86RF215_SPI_CSN_PIN_MASK    GPIO_PIN_MASK(AT86RF215_SPI_CSN_PIN)
#define AT86RF215_RSTN_PORT_BASE      GPIO_PORT_TO_BASE(AT86RF215_RSTN_PORT)
#define AT86RF215_RSTN_PIN_MASK       GPIO_PIN_MASK(AT86RF215_RSTN_PIN)
#define AT86RF215_PWR_PORT_BASE       GPIO_PORT_TO_BASE(AT86RF215_PWR_PORT)
#define AT86RF215_PWR_PIN_MASK        GPIO_PIN_MASK(AT86RF215_PWR_PIN)
#define AT86RF215_IRQ_PORT_BASE       GPIO_PORT_TO_BASE(AT86RF215_IRQ_PORT)
#define AT86RF215_IRQ_PIN_MASK        GPIO_PIN_MASK(AT86RF215_IRQ_PIN)
/*---------------------------------------------------------------------------*/
extern void at86rf215_isr(void);
/*---------------------------------------------------------------------------*/
static void
at86rf215_interrupt_handler(gpio_hal_pin_mask_t pin_mask)
{
    at86rf215_isr();
}
/*---------------------------------------------------------------------------*/
static gpio_hal_event_handler_t irq_handler = {
  .next = NULL,
  .handler = at86rf215_interrupt_handler,
  .pin_mask = gpio_hal_pin_to_mask(AT86RF215_IRQ_PIN) << (AT86RF215_IRQ_PORT << 3),
};
/*---------------------------------------------------------------------------*/
void 
at86rf215_arch_enable_EXTI(void)
{
    GPIO_ENABLE_INTERRUPT(AT86RF215_IRQ_PORT_BASE, AT86RF215_IRQ_PIN_MASK);
    NVIC_EnableIRQ(AT86RF215_GPIOx_VECTOR);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_disable_EXTI(void)
{
    GPIO_DISABLE_INTERRUPT(AT86RF215_IRQ_PORT_BASE, AT86RF215_IRQ_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_set_RSTN(void)
{   
    GPIO_CLR_PIN(AT86RF215_RSTN_PORT_BASE, AT86RF215_RSTN_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_clear_RSTN(void)
{
    GPIO_SET_PIN(AT86RF215_RSTN_PORT_BASE, AT86RF215_RSTN_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_spi_select(void)
{   
    GPIO_CLR_PIN(AT86RF215_SPI_CSN_PORT_BASE, AT86RF215_SPI_CSN_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
void
at86rf215_arch_spi_deselect(void)
{
    GPIO_SET_PIN(AT86RF215_SPI_CSN_PORT_BASE, AT86RF215_SPI_CSN_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
uint8_t 
at86rf215_arch_spi_txrx(uint8_t b)
{
    SPIX_WAITFORTxREADY(AT86RF215_SPI_INSTANCE);
    SPIX_BUF(AT86RF215_SPI_INSTANCE) = b;
    SPIX_WAITFOREOTx(AT86RF215_SPI_INSTANCE);
    SPIX_WAITFOREORx(AT86RF215_SPI_INSTANCE);
    b = SPIX_BUF(AT86RF215_SPI_INSTANCE);
    return b;
}
/*---------------------------------------------------------------------------*/
uint8_t 
at86rf215_arch_spi_fast_read(uint8_t a1, uint8_t a2){
    uint8_t b;
    GPIO_CLR_PIN(AT86RF215_SPI_CSN_PORT_BASE, AT86RF215_SPI_CSN_PIN_MASK);
    SPIX_BUF(AT86RF215_SPI_INSTANCE) = a1;
    SPIX_WAITFOREOTx(AT86RF215_SPI_INSTANCE);
    b = SPIX_BUF(AT86RF215_SPI_INSTANCE);
    SPIX_BUF(AT86RF215_SPI_INSTANCE) = a2;
    SPIX_WAITFOREOTx(AT86RF215_SPI_INSTANCE);
    b = SPIX_BUF(AT86RF215_SPI_INSTANCE);
    SPIX_BUF(AT86RF215_SPI_INSTANCE) = 0x00;
    SPIX_WAITFOREOTx(AT86RF215_SPI_INSTANCE);
    b = SPIX_BUF(AT86RF215_SPI_INSTANCE);
    GPIO_CLR_PIN(AT86RF215_SPI_CSN_PORT_BASE, AT86RF215_SPI_CSN_PIN_MASK);
    return b;
}

/*---------------------------------------------------------------------------*/
void
at86rf215_arch_init(void)
{
    /* Initialize power pin */
    GPIO_SOFTWARE_CONTROL(AT86RF215_PWR_PORT_BASE , AT86RF215_PWR_PIN_MASK );
    GPIO_SET_OUTPUT(AT86RF215_PWR_PORT_BASE , AT86RF215_PWR_PIN_MASK );
    
    /* Initialize reset pin */
    GPIO_SOFTWARE_CONTROL(AT86RF215_RSTN_PORT_BASE ,AT86RF215_RSTN_PIN_MASK );
    GPIO_SET_OUTPUT(AT86RF215_RSTN_PORT_BASE ,AT86RF215_RSTN_PIN_MASK );

    /* Turn the radio off */
    GPIO_CLR_PIN(AT86RF215_PWR_PORT_BASE , AT86RF215_PWR_PIN_MASK );
    GPIO_CLR_PIN(AT86RF215_RSTN_PORT_BASE ,AT86RF215_RSTN_PIN_MASK );
    for(uint16_t delay=0; delay<0xA2C2; delay++);

    /* Power up the radio */
    GPIO_SET_PIN(AT86RF215_PWR_PORT_BASE , AT86RF215_PWR_PIN_MASK );
    for(uint16_t delay=0; delay<0xA2C2; delay++);

    /* "Un-reset" the radio */
    at86rf215_arch_clear_RSTN();

    /* Initialize SPI */
    spix_init(AT86RF215_SPI_INSTANCE);
    spix_cs_init(AT86RF215_SPI_CSN_PORT, AT86RF215_SPI_CSN_PIN);
    spix_set_mode(AT86RF215_SPI_INSTANCE, SSI_CR0_FRF_MOTOROLA, 0, 0, 8);

    /* AT86RF215 supports SPI clock up to 25MHz, but CC2538 supports max 16MHz
     * by using 32MHz CPU clock (defined in board.h) */
    spix_set_clock_freq(AT86RF215_SPI_INSTANCE, 16000000);

    /* Disable SPI interrupts - to reduce the time needed for read/write execution */
    NVIC_DisableIRQ(SSI0_IRQn);
    REG(SSI0_BASE + SSI_IM) = 0x0F;


    /* Initialize IRQs */
    GPIO_SOFTWARE_CONTROL(AT86RF215_IRQ_PORT_BASE, AT86RF215_IRQ_PIN_MASK);
    GPIO_SET_INPUT(AT86RF215_IRQ_PORT_BASE, AT86RF215_IRQ_PIN_MASK);
    ioc_set_over(AT86RF215_IRQ_PORT, AT86RF215_IRQ_PIN, IOC_OVERRIDE_PUE);        // Pull-up enable

    GPIO_DETECT_EDGE(AT86RF215_IRQ_PORT_BASE, AT86RF215_IRQ_PIN_MASK);
    GPIO_TRIGGER_SINGLE_EDGE(AT86RF215_IRQ_PORT_BASE, AT86RF215_IRQ_PIN_MASK);
    GPIO_DETECT_FALLING(AT86RF215_IRQ_PORT_BASE, AT86RF215_IRQ_PIN_MASK);

    /* Set radio priority higher than rtimer - so isr can execute durring tx/rx routine*/
    NVIC_SetPriority(SMT_IRQn, 1);
    NVIC_SetPriority(GPT1A_IRQn, 1);
    NVIC_SetPriority(UART0_IRQn, 1);    // So that radio ISR has higher priority than UART ... TODO remove when end of development
    NVIC_SetPriority(AT86RF215_GPIOx_VECTOR, 0);

    gpio_hal_register_handler(&irq_handler);
}
