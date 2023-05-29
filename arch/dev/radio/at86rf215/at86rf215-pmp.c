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
 * @brief Phase Measurement Process functions for AT86RF215 on OpenMoteB.
 *
 *        Most of the functions are architecture dependent, so they are moved 
 *        to a separate file instead of being alongside the radio drivers.
 *
 * @author Grega Morano <grega.morano@ijs.si>
 */
/*---------------------------------------------------------------------------*/
#include "dev/gpio.h"
#include "dev/gpio-hal.h"
#include "at86rf215-registermap.h"
#include "at86rf215-arch.h"
#include "at86rf215-conf.h"
#include "at86rf215-pmp.h"


#if 1
#define PMP_DEBUG_EVENT(s)                                              \
({                                                                      \
    if(s){                                                              \
        GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(3));  \
    }else{                                                              \
        GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(3));  \
    }                                                                   \
})
#else
#define PMP_DEBUG_EVENT(s) 
#endif


/*---------------------------------------------------------------------------*/
PROCESS(at86rf215_print_process, "AT86RF215 print");
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/**
 * @brief Control the proprietary uniform circular antenna (UCA) array.
 *
 * Function maps antenna numbers from 0 to 11 to antennas 1_1 to 3_4
 * Default antenna is selected with either 0 or -1.
 *
 * Connection:
 * B0  --> CTRL 1_1
 * B1  --> CTRL 1_2
 * B2  --> CTRL 2_1
 * B3  --> CTRL 2_2
 *
 * Function duration --> 1.8us
 */
void
UCA_select_antenna_element(int8_t antenna)
{
    /* Default is antenna 1_1 */
    if(antenna < 0){
        GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(0));
        GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(1));
        GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(2));
        GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(3));
        return;
    }
    /* Mapping */
    if(antenna < 6){
        GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(0));
    }
    else {
        GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(0));
        antenna -= 6;
    }
    if(antenna > 2){
        GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(1));
        antenna -= 3;
    }
    else{
        GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(1));
    }
    if(antenna == 2){
        GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(2));
    }
    else{
        GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(2));
    }
    if(antenna > 0){
        GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(3));
    }
    else{
        GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(3));
    }
}

/*---------------------------------------------------------------------------*/
/* Fast SPI functions */
/*---------------------------------------------------------------------------*/
#include "dev/spi-arch-legacy.h"
#include "dev/spi-legacy.h"
#include "dev/ioc.h"

/**
 * \note Architecture depended - used for OpenMoteB only.
 *
 * Normal SPI read/write operations are slow (up to 8us for single register
 * read). We implemented an example of faster approach with
 * at86rf215_arch_spi_fast_read() function. But fore some reason (most likely
 * complier/stack) it requires up to 1us to enter the function. This 
 * is too much for our needs of 1-2us SPI read/write operations.
 *
 * By using inline assembly we can reduce the time ~4us for single register
 * read operation. Even though this are architecture dependent functions,
 * we put them here to separate them from normal radio operation.
 * Needed only for PMP to work properly.
 * 
 * regRead() --> 8us
 * regRead_fast() --> 3.8us
 * readPhase_fast() --> 3.6us (200ns reduced by defining the address to read)
 * readPhaseQf_fast() --> 4.8us (read 2 registers)
 * 
 * burstRead_fast(len=5) --> 9.4us
 * readPmu_fast(len=5) --> 8.9us
 * 
 * regWrite() --> 7.9us
 * regWrite_fast() --> 3.8us
 * 
 * Execution times also heavily depend on the code structure and compiler 
 * optimization level. Sometimes it works best with -O2, sometimes with -Os...
 * To optimize just a single function, you can use:
 * __attribute__((optimize("Os")))
 *
 * In addition, the AT86RF215 supports max SPI speed of 25MHz, but CC2538
 * supports only 16MHz (and this only by using 32MHz CPU clock instead of the
 * default 16MHz)...

 * The SPI interrupts are also disabled (in arch) to reduce the overhead.
 */
/*---------------------------------------------------------------------------*/
static inline uint8_t
regRead_fast(uint16_t address)
{
    uint8_t data;
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_READ | (uint8_t)((address & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(address & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    data = SPIX_BUF(0);
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    return data;
}
/*---------------------------------------------------------------------------*/
static inline void
regWrite_fast(uint16_t address, uint8_t data)
{
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_WRITE | (uint8_t)((address & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(address & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = data;
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
}
/*---------------------------------------------------------------------------*/
static inline void
burstRead_fast(uint16_t address, uint8_t *data, uint8_t len)
{
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_READ | (uint8_t)((address & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(address & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    for(uint8_t i=0; i<len; i++) {
        SPIX_BUF(0) = 0x00;
        SPIX_WAITFOREOTx(0);
        data[i] = SPIX_BUF(0);
    }
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
}
/*---------------------------------------------------------------------------*/
static inline void
burstWrite_fast(uint16_t address, uint8_t *data, uint8_t len)
{
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_WRITE | (uint8_t)((address & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(address & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    for(uint8_t i=0; i<len; i++) {
        SPIX_BUF(0) = data[i];
        SPIX_WAITFOREOTx(0);
        SPIX_BUF(0);
    }
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
}
/*---------------------------------------------------------------------------*/
static inline uint8_t
pmu_get_sync(uint8_t *sync)
{
    uint8_t b;
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_READ | (uint8_t)((RG_BBC1_PMUC & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(RG_BBC1_PMUC & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    b = SPIX_BUF(0);
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    sync[1] = (b & 0x1C) >> 2;
    return ((b & 0x1C) >> 2);
}
/*---------------------------------------------------------------------------*/
static inline void
pmu_get_phase(uint8_t *phase)
{
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_READ | (uint8_t)((RG_BBC1_PMUVAL & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(RG_BBC1_PMUVAL & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    phase[0] = SPIX_BUF(0);
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
}
/*---------------------------------------------------------------------------*/
static inline void __attribute__((optimize("Os")))
pmu_get_phase_sync(uint8_t *phase)
{
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_READ | (uint8_t)((RG_BBC1_PMUC & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(RG_BBC1_PMUC & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    phase[0] = SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    phase[1] = SPIX_BUF(0);
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
}
/*---------------------------------------------------------------------------*/
static inline void __attribute__((optimize("Os"))) // Lasts for 6.7 us
pmu_get_phase_sync_qf(uint8_t *phase)
{
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_READ | (uint8_t)((RG_BBC1_PMUC & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(RG_BBC1_PMUC & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    phase[0] = SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    phase[1] = SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    phase[2] = SPIX_BUF(0);
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
}
/*---------------------------------------------------------------------------*/
static inline void __attribute__((optimize("Os")))
pmu_get_IQ(uint8_t *phase)
{
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_READ | (uint8_t)((RG_BBC1_PMUI & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(RG_BBC1_PMUI & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    phase[0] = SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    phase[1] = SPIX_BUF(0);
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
}
/*---------------------------------------------------------------------------*/
/* 5B Lasts for apx 9us, while 4B only for 7.9 us, while 3B only for 7us */
static inline void __attribute__((optimize("Os")))
pmu_get_all(uint8_t *pmu)
{
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
    SPIX_BUF(0) = (SPI_CMD_READ | (uint8_t)((RG_BBC1_PMUC & 0xFF00) >> 8));
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = (uint8_t)(RG_BBC1_PMUC & 0x00FF);
    SPIX_WAITFOREOTx(0);
    SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    pmu[0] = SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    pmu[1] = SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    pmu[2] = SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    pmu[3] = SPIX_BUF(0);
    SPIX_BUF(0) = 0x00;
    SPIX_WAITFOREOTx(0);
    pmu[4] = SPIX_BUF(0);
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3));
}




/*---------------------------------------------------------------------------*/
/* Periodic 16 us timer implementation */
/*---------------------------------------------------------------------------*/
#include "reg.h"
#include "cpu.h"
#include "dev/gptimer.h"
#include "dev/sys-ctrl.h"
/**
 * \note Architecture depended - used for OpenMoteB only.
 *
 * \brief Timer for Phase Measurement Process
 *        used to create exact 16 us intervals by using GP Timer A3 counting
 *        up to the value 512, with sys clock of 32 MHz.
 */
/*---------------------------------------------------------------------------*/

/* Global var that indicates timeout event - timer has elapsed */
volatile uint8_t pmp_timer_triggered = 0;

/*---------------------------------------------------------------------------*/
void pmp_timer_init(void)
{
    /* Remove the clock gate to enable GPT3 and then initialise it*/
    REG(SYS_CTRL_RCGCGPT) |= SYS_CTRL_RCGCGPT_GPT3;

    /* Make sure GPT3 is off */
    REG(GPT_3_BASE + GPTIMER_CTL) = 0x00;

    /* Use only timer A (16-bit is enough) */
    REG(GPT_3_BASE + GPTIMER_CFG) = 0x04;

    /* Periodic, Count Up */
    REG(GPT_3_BASE + GPTIMER_TAMR) = 0x0012;

    /* Enable Time-out event interrupt */
    REG(GPT_3_BASE + GPTIMER_IMR) = 0x0001;

    /* Preload value for periodic timer: 16us == 512*/
    REG(GPT_3_BASE + GPTIMER_TAILR) = 512;

    /* Don't enable the counter just yet */
    //REG(GPT_3_BASE + GPTIMER_CTL) = 0x01;
}
/*---------------------------------------------------------------------------*/
void pmp_timer_enable(void)
{
    INTERRUPTS_DISABLE();

    pmp_timer_triggered = 0;

    /* Clear status flag */
    REG(GPT_3_BASE + GPTIMER_ICR) = 0xFF;

    INTERRUPTS_ENABLE();

    NVIC_ClearPendingIRQ(GPT3A_IRQn);
    NVIC_EnableIRQ(GPT3A_IRQn);

    /* Start the timer */
    REG(GPT_3_BASE + GPTIMER_CTL) = 0x0001;
}
/*---------------------------------------------------------------------------*/
void pmp_timer_disable(void)
{
    /* Disable the timer */
    REG(GPT_3_BASE + GPTIMER_CTL) = 0x0000;

    /* Clear status flag */
    REG(GPT_3_BASE + GPTIMER_ICR) = 0xFF;

    NVIC_ClearPendingIRQ(GPT3A_IRQn);
    NVIC_DisableIRQ(GPT3A_IRQn);

}
/*---------------------------------------------------------------------------*/
/**
 * \note The ISR function must be registered at arc/cpu/cc2538/startup-gcc.c
 */
void pmp_timer_isr()
{
    REG(GPT_3_BASE + GPTIMER_ICR) = 0xFF;
    pmp_timer_triggered = 1;
    NVIC_ClearPendingIRQ(GPT3A_IRQn);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Delay for certain a amount - develop purpose
 * 
 *  t  == true (measured) delay 
 *
 *  17 == 8.20
 *  13 == 6.68
 *  12 == 6.20         
 *   7 == 4.16      
 *   5 == 3.36
 *   3 == 2.54
 */
static inline void __attribute__((optimize("O0")))
__delay(uint8_t t){
    for(uint8_t i=0; i<t; i++);
}



/*---------------------------------------------------------------------------*/
/* The Phase Measurement Process (PMP)  */
/*---------------------------------------------------------------------------*/
/**
 * \brief Fast implementation of set_frequency().
 *
 * \param freq - frequency in MHz (e.g. 2405)
 * \param decimal - frequency in 100kHz (e.g. 6)
 */
static inline void
set_frequency_fast(uint16_t freq, uint8_t decimal)
{
    uint32_t N = ((freq - 2366) * 65536) / 26;

    N += decimal * 252;

    if(N > 296172){
        N = 296172;
    }
    else if(N < 85700){
        N = 85700;
    }
    uint8_t data[4]={
        (uint8_t)((N & 0xFF00) >> 8),
        (uint8_t)((N & 0xFF0000) >> 16),
        (uint8_t)( N & 0x00FF),
        0xC0
    };
    burstWrite_fast(RG_RF24_CCF0L, data, 4);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Same as set_frequency_fast, just that you can add artificial CFO to 
 * the desired frequency.
 *
 * \param cfo - desired CFO
 * value of 1 equals 396 Hz
 * value of 3 equals 1.2 kHz
 * value of 25 equals 9.9 kHz
 * value of 76 equals 30.1 kHz
 */
static inline void
at86rf215_set_frequency_fast_cfo(uint16_t freq, uint8_t decimal, uint8_t cfo)
{
    uint32_t N = ((freq - 2366) * 65536) / 26;

    N += decimal * 252;

    N += cfo;

    if(N > 296172){
        N = 296172;
    }
    else if(N < 85700){
        N = 85700;
    }
    uint8_t data[4]={
        (uint8_t)((N & 0xFF00) >> 8),
        (uint8_t)((N & 0xFF0000) >> 16),
        (uint8_t)( N & 0x00FF),
        0xC0
    };
    burstWrite_fast(RG_RF24_CCF0L, data, 4);
}


/*---------------------------------------------------------------------------*/
/**
 * 
 *
 * Synchronization - is done by the Contiki-NG. However, due to limitations of
 * rtimers accuracy, the devices start the PM process at different times - appx
 * 1 rtimer tick out of sync. The default rtimer defined in cc2538 has a slow
 * clock and results in: 1 tick --> 30.5 us.
 *
 *
 *
*/

#define IF_FREQ         (1)     // 1 - 1 MHz, 2 - 2MHz, default 0.5 MHz

#define PMU_CCFTS       (0)     // (0) - Car. freq. changes right away 1 - Change car. freq. at end of PMU period
#define PMU_IQSEL       (0)     // (0) - Normalized,                   1 - Without normalization
#define PMU_FED         (0)     // (0) - Quality Factor,               1 - Frequency Error Detection
#define PMU_AVG         (0)     //  0 - Sample at the end,            (1) - Average in between
#define PMU_EN          (1)     // (0) - Disable,                      1 - Enable

#define PMU_CONFIG (PMU_CCFTS<<7 | PMU_IQSEL<<6 | PMU_FED<<5 | PMU_AVG<<1 | PMU_EN<<0)

#define SWITCH_PATTERN  (0)     // (0) - single antenna,    1 - Round Robin,      2 - Return-t-first

/*
 * value of 1 equals 396 Hz
 * value of 3 equals 1.2 kHz
 * value of 25 equals 9.9 kHz
 * value of 76 equals 30.1 kHz
 * 150 = 60 kHz
 * 250 = 100 kHz
 */
#define CFO             25

#define NUM_OF_SLOTS    90


static uint8_t phases[294] = {0};   // 90 slots * 3 bytes = 270 + 3*8B @ preamble
static uint8_t chn = 0;

int
at86rf215_phase_measurement_process(unsigned char  role, unsigned char  channel, unsigned char  *phase)
{
    chn = channel;
    uint16_t freq_i = 2405 + 5 * (channel - 11);
    uint16_t freq_r = 2405 + 5 * (channel - 11);
    uint8_t freq_off = 0;

    // Setup - config ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    PMP_DEBUG_EVENT(1);
   
    /* Disable IRQs */
    at86rf215_arch_disable_EXTI();

    /* Go to TRX OFF state */
    regWrite_fast(RG_RF24_CMD, RF_CMD_TRXOFF);

    /* RX RF analog frontend config */
#if IF_FREQ == 1
    regWrite_fast(RG_RF24_RXBWC, 0x06);      // IF = 1MHz, BBF BW = najmanjsi mozen 630kHz
    regWrite_fast(RG_RF24_RXDFE, 0x84);      // SR = 1MHz
    freq_i -= 1;
    freq_off = 0;
#elif IF_FREQ == 2
    regWrite_fast(RG_RF24_RXBWC, 0x09);      // IF = 2MHz, BBF BW = najmanjsi mozen
    regWrite_fast(RG_RF24_RXDFE, 0x82);      // SR = 2MHz
    freq_i -= 2;
    freq_off = 0;
#else
    regWrite_fast(RG_RF24_RXBWC, 0x05);      // IF = 500kHz, BBF BW = najmanjsi mozen
    regWrite_fast(RG_RF24_RXDFE, 0x88);      // SR = 0.5MHz
    freq_i -= 1;
    freq_off = 5;
#endif

    /* RX digital frontend config */
    regWrite_fast(RG_RF24_AGCC, 0x00);       // Disable AGC (AGCC.EN = 0) to enable manual control of the gain
    regWrite_fast(RG_RF24_AGCS, 0x77);       // set the gain (AGCS.GCW) - Max gain is 0x77, med is 0x6B, min is 0x00
    regWrite_fast(RG_RF24_EDC, 0x03);        // Disable ED measurement (EDC.EN = 0)

    /* TX RF analog frontend config */
    regWrite_fast(RG_RF24_TXCUTC, 0x08);     // neki | LPF 500 KHz
    /* TX digital frontend config */
    regWrite_fast(RG_RF24_TXDFE, 0x81);      // fcut = 1 * fs/2 | direct modulatio = 0 | SR = 4MHz
    regWrite_fast(RG_RF24_PAC, 0xFF);        // no current reduction & Set max out power: 0x1F = 31 = 15dBm

    /* Configure and enable PMU */
    regWrite_fast(RG_BBC1_PMUC, PMU_CONFIG);  

    /* Go to TXPREP state */
    regWrite_fast(RG_RF24_CMD, RF_CMD_TXPREP);
    clock_delay_usec(100);                  // Transition time required by the radio (TRXOFF --> TXPREP)

    PMP_DEBUG_EVENT(0);



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(role == 0){  /* INITIATOR */

        uint8_t slot_num = 0;
        uint8_t *phases_with_pre = phases + 24;

        /* Select desired freq. */
        at86rf215_set_frequency_fast_cfo(freq_i,freq_off, CFO);
        clock_delay_usec(20);                               // Wait for PLL lock (usually faster than 100us, appx 20us)

        /* Enable baseband and disable CTX */
        regWrite_fast(RG_RF24_CMD, RF_CMD_RX);              // Prepare for RX
        clock_delay(80);                                    // wait for state transfer to RX
        

        __delay(2);
        pmp_timer_enable();


        // Guard time
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //while(!pmp_timer_triggered);
        //pmp_timer_triggered = 0;


        // Preamble
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        for (uint8_t pre = 0; pre<8; pre++){
            pmu_get_phase_sync_qf(phases+(pre*3));
            pre++;
            __delay(3);
            pmu_get_phase_sync_qf(phases+(pre*3));
            while(!pmp_timer_triggered);
            pmp_timer_triggered = 0;
        }


        // Sample-Switch
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if SWITCH_PATTERN == 1 /* ROUND ROBIN */ //////////////////////////////////////////////////////////////////////////////////////////////////////

        for (uint8_t cycle = 0; cycle < 7; cycle++){
            for(uint8_t a=0; a<12; a++){
                while(!pmp_timer_triggered);
                pmp_timer_triggered = 0;
                pmu_get_phase_sync_qf(phases_with_pre+(slot_num*3));
                UCA_select_antenna_element((a+1)%12);
                slot_num +=1;
            }
        }

#elif SWITCH_PATTERN == 2 /* RETURN TO FIRST */ /////////////////////////////////////////////////////////////////////////////////////////////////

        for (uint8_t cycle = 0; cycle < 4; cycle++){
            for(uint8_t a=1; a<12; a++){

                while(!pmp_timer_triggered);
                pmp_timer_triggered = 0;

                /* Sample on antenna 1 and select the following one */
                pmu_get_phase_sync_qf(phases_with_pre+(slot_num*3));
                UCA_select_antenna_element(a);
                slot_num +=1;

                while(!pmp_timer_triggered);
                pmp_timer_triggered = 0;

                /* Sample on the a-th antenna and then select antenna 0 */
                pmu_get_phase_sync_qf(phases_with_pre+(slot_num*3));
                UCA_select_antenna_element(-1);
                slot_num +=1;
            }
        }
    
#else   /* SINGLE ANTENNA, 90 samples */ //////////////////////////////////////////////////////////////////////////////////////////////////
        for(slot_num = 0; slot_num < NUM_OF_SLOTS; slot_num++){
            while(!pmp_timer_triggered);
            pmp_timer_triggered = 0;
            PMP_DEBUG_EVENT(1);
            pmu_get_phase_sync_qf(phases_with_pre+(slot_num*3));
            PMP_DEBUG_EVENT(0);
        }
#endif  /* SWITCH_PATTERN */

        pmp_timer_disable();

        UCA_select_antenna_element(-1);

        process_poll(&at86rf215_print_process);

    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else{           /* REFLECTOR */

            at86rf215_set_frequency_fast_cfo(freq_r, 0, 0);
            clock_delay_usec(20);


            /* Disable baseband and enable CTX */
            regWrite_fast(RG_RF_IQIFC1,0x12);
            regWrite_fast(RG_RF24_TXDACI, 0xFE);
            regWrite_fast(RG_RF24_TXDACQ, 0xFE);

            PMP_DEBUG_EVENT(1);

            /* Start with the CW and wait for other to measure */
            regWrite_fast(RG_RF24_CMD, RF_CMD_TX);
            clock_delay_usec(1600);

            PMP_DEBUG_EVENT(0);

            regWrite_fast(RG_RF24_CMD, RF_CMD_TXPREP);
            clock_delay_usec(10);                               // Wait for state transition
        
    }

    // Restore config ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //rtimer_clock_t start = RTIMER_NOW();
    
    /* Restore the register setup */
    regWrite_fast(RG_RF24_CMD, RF_CMD_TRXOFF);

    /* Disable DAC overwrite, disable PMU and enable BBC */
    regWrite_fast(RG_RF24_TXDACI, 0x00);
    regWrite_fast(RG_RF24_TXDACQ, 0x00);
    regWrite_fast(RG_BBC1_PMUC, 0x00);
    regWrite_fast(RG_RF_IQIFC1, 0x02); 

    for (uint8_t i = 0; i < (sizeof(AT86RF215_RADIO_CONFIGURATION)/sizeof(registerSetting_t)); i++) {
        regWrite_fast(AT86RF215_RADIO_CONFIGURATION[i].address, AT86RF215_RADIO_CONFIGURATION[i].value);
    }
    at86rf215_set_frequency_fast_cfo(freq_r, 0, 0);
    regWrite_fast(RG_BBC1_PC, 0x1F);

    /* Enable IRQ and clear all 4 IRQs */
    uint8_t dummy[4] __attribute__((unused));
    burstRead_fast(RG_RF09_IRQS, dummy, 4);
    at86rf215_arch_enable_EXTI();

    /* Put the radio back to RX - to receive the ACK*/
    regWrite_fast(RG_RF24_CMD, RF_CMD_RX);
    clock_delay_usec(70);

    //printf("Duration %d\n", RTIMER_NOW() - start);
    return 1;
}


/*---------------------------------------------------------------------------*/
/**
 * @brief Print the measurements from the PM Process
 *
 *  The time it takes for the nodes to print out the results (300 B) is too 
 *  long to be printed within the timeslot (it requires up to 16ms to transmit
 *  the buffer over the UART). For small measurements (up to 60B) we can print 
 *  them using the Contiki-NG TSCH_LOG_ADD module. For longer ones, use the
 *  process below. 
 */
PROCESS_THREAD(at86rf215_print_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    printf("CH = %d\n", chn);
    printf("PH = [\n");
    for(uint16_t i=0; i<294; i+=3){

        printf("%d, %d, (%d), \n", ((phases[i] & 0x1C) >> 2), phases[i+1], phases[i+2]);
        if(i == 21){
            printf("\n");   // add space after preamble period
        }
    }
    printf("]\n");
  }
  PROCESS_END();
}







