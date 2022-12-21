#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rf233_pmp.h"
#include "rf2xx.h"
#include "rf2xx_registermap.h"
#include "rf2xx_hal.h"
#include "sys/log.h"

#include "stm32f10x_gpio.h"

#define LOG_MODULE  "PMP"
#define LOG_LEVEL   LOG_LEVEL_INFO

#if AT86RF233

// Golomb ruler
// Thanks to: O. Oshiga, A. Ghods, S. Severi, and G. Abreu, “Efficient Slope Sampling Ranging and Trilateration Techniques for Wireless Localization”
uint8_t  golomb_ruler[] = {1, 12, 15, 16, 25, 46, 62, 85, 104, 121, 126, 133, 153, 159, 161};
uint16_t golomb_freq[] = {2400, 2406, 2407, 2408, 2412, 2423, 2431, 2442, 2452, 2460, 2463, 2466, 2476, 2479, 2480};
uint8_t  golomb_offset[] = {1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1};


#if PMP_GPIO_DEBUG
	#define VESNA_GPIO(state)                   \
        if(state){                              \
            GPIO_SetBits(GPIOA, GPIO_Pin_5);    \
        } else{                                 \
            GPIO_ResetBits(GPIOA, GPIO_Pin_5);  \
        }
#else
	#define VESNA_GPIO(state) 
#endif

/*------------------------------------------------------------------------------------------*/
void
rf233_register_backup(void){
    bkp_register.trx_ctrl_0  = regRead(RG_TRX_CTRL_0);
    bkp_register.trx_ctrl_1  = regRead(RG_TRX_CTRL_1);
    bkp_register.trx_ctrl_2  = regRead(RG_TRX_CTRL_2);
    bkp_register.phy_tx_pwr  = regRead(RG_PHY_TX_PWR);
    bkp_register.xah_ctrl_0  = regRead(RG_XAH_CTRL_0);
    bkp_register.xah_ctrl_1  = regRead(RG_XAH_CTRL_1);
    bkp_register.cc_ctrl_0   = regRead(RG_CC_CTRL_0);
    bkp_register.cc_ctrl_1   = regRead(RG_CC_CTRL_1);
    bkp_register.csma_seed_0 = regRead(RG_CSMA_SEED_0);
    bkp_register.csma_be     = regRead(RG_CSMA_BE);
    bkp_register.tst_sdm     = regRead(RG_TST_SDM);
    bkp_register.tst_agc     = regRead(RG_TST_AGC);
    bkp_register.rx_syn      = regRead(RG_RX_SYN);
}
/*------------------------------------------------------------------------------------------*/
void
rf233_register_restore(void){
    regWrite(RG_TRX_CTRL_0, bkp_register.trx_ctrl_0);
    regWrite(RG_TRX_CTRL_1, bkp_register.trx_ctrl_1);
    regWrite(RG_TRX_CTRL_2, bkp_register.trx_ctrl_2);
    regWrite(RG_PHY_TX_PWR, bkp_register.phy_tx_pwr);
    regWrite(RG_XAH_CTRL_0, bkp_register.xah_ctrl_0);
    regWrite(RG_XAH_CTRL_1, bkp_register.xah_ctrl_1);
    regWrite(RG_CC_CTRL_0,  bkp_register.cc_ctrl_0);
    regWrite(RG_CC_CTRL_1,  bkp_register.cc_ctrl_1);
    regWrite(RG_CSMA_SEED_0,bkp_register.csma_seed_0);
    regWrite(RG_CSMA_BE,    bkp_register.csma_be);
    regWrite(RG_TST_SDM,    bkp_register.tst_sdm);
    regWrite(RG_TST_AGC,    bkp_register.tst_agc);
    regWrite(RG_RX_SYN,     bkp_register.rx_syn);

    regWrite(RG_IRQ_MASK, DEFAULT_IRQ_MASK);    
    regRead(RG_IRQ_STATUS);                         // Clear any pending interrupts
}
/*------------------------------------------------------------------------------------------*/
void rf233_prepare_for_PMP(void){

    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);  // Turn the radio in to off state
    rf233_register_backup();                        // Store all of the register in use

    regRead(RG_IRQ_STATUS);
    regWrite(RG_IRQ_MASK, 0x00);                    // Disable all IRQs from the radio  
    regWrite(RG_TRX_CTRL_0, 0x24);                  // Enable PMU unit, disable TOM, CLKM_SEL=1, CLKM_CTRL=4
    bitWrite(SR_TX_PWR, PMP_CW_POWER);              // Set output power (max = 0x00)
    bitWrite(SR_TX_AUTO_CRC_ON, 0);                 // Disable AUTO_CRC_ON
    bitWrite(SR_RX_PDT_DIS, 1);                     // Disable RX path
    bitWrite(SR_AGC_OFF, 1);                        // Turn off automatic gain control
    bitWrite(SR_MOD_SEL, 1);		                // Manual control of modulation data
    bitWrite(SR_MOD, 0);			                // Continuous 0 chips
    bitWrite(SR_TX_RX_SEL, 1);		                // Manual control of PLL frequency mode

    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_PLL_ON);   // Go to RX_ON (via PLL_ON)
    vsnTime_delayUS(80);                            // Wait for state transition (110 us, but below..)
}
/*------------------------------------------------------------------------------------------*/
void
rf233_set_frequency(uint16_t f, uint8_t o){

        if (f < 2434){
            regWrite(RG_CC_CTRL_1, 0x08);
            f -= 2306;
        }
        else{
            regWrite(RG_CC_CTRL_1, 0x09);
            f -= 2434;
        }
        f *= 2;
        if(o){
            f += 1;
        }
        regWrite(RG_CC_CTRL_0, (uint8_t) f);
}
/*------------------------------------------------------------------------------------------*/
void
rf233_transmit(uint32_t duration){
    VESNA_GPIO(1);
    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_PLL_ON);
    regWrite(RG_TRX_STATE, TRX_CMD_TX_START);
    vsnTime_delayUS(duration);
    VESNA_GPIO(0);
}
/*------------------------------------------------------------------------------------------*/
void 
rf233_read_phase(uint8_t *data){
    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_PLL_ON); 
    regWrite(RG_TRX_STATE, TRX_CMD_RX_ON); 
    VESNA_GPIO(1);
    *data = regRead(RG_PHY_PMU_VALUE);
    VESNA_GPIO(0);
}

/*------------------------------------------------------------------------------------------*/
int 
rf233_phase_measurement_process(uint8_t role, uint8_t channel, uint8_t *phase){

    //uint16_t start = RTIMER_NOW();
    uint16_t freq, freq_f, freq_o;

    uint8_t phases[PMP_MEASUREMENT_SIZE] = {0};    

    rf233_prepare_for_PMP();                            // Prepare the radio for PMP
    
    if(role == 0){  // INITIATOR

        bitWrite(SR_PMU_IF_INVERSE, 0);                 // Disable inverse IF
        bitWrite(SR_TX_RX, 0);                          // RX PLL frequency = 0
        vsnTime_delayUS(20);                            // Sync delay (might differ for different uC)

        for(uint8_t f=0; f<PMP_MEASUREMENT_SIZE; f++){
            freq = golomb_ruler[f];
            freq_f = freq / 2;
            freq_o = freq & 0x0001;
            
            rf233_set_frequency((PMP_FIRST_CHANNEL + freq_f), freq_o);
            vsnTime_delayUS(16);                        // Allow freq to settle (from datasheet)
            vsnTime_delayUS(PMP_GUARD_TIME);            // Guard time delay
            rf233_read_phase(phases + f);               // Read phase sample
            vsnTime_delayUS(PMP_TRANSITION_TIME);       // Wait for other
            rf233_transmit(PMP_GUARD_TIME + 60);        // Transmit CW
        }      
    }
    else            // REFLECTOR
    {                                           
        bitWrite(SR_PMU_IF_INVERSE, 1);                 // Enable inverse IF
        bitWrite(SR_TX_RX, 1);                          // TX PLL frequency = 1

        for(uint8_t f=0; f<PMP_MEASUREMENT_SIZE; f++){
            freq = golomb_ruler[f] + 1;                 // Add 0.5 MHz
            freq_f = freq / 2;
            freq_o = freq & 0x0001;
            
            rf233_set_frequency((PMP_FIRST_CHANNEL + freq_f), freq_o);
            vsnTime_delayUS(16);                        // Allow freq to settle (from datasheet)
            rf233_transmit(PMP_GUARD_TIME + 60);        // Transmit CW
            vsnTime_delayUS(PMP_TRANSITION_TIME);       // Wait for the other
            rf233_read_phase(phases + f);               // Read phase sample
            vsnTime_delayUS(PMP_GUARD_TIME);            // Guard time delay (to be in sync with the other)
        }
    }

    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);      
    rf233_register_restore();                           // Restore the radio registers
    regWrite(RG_TRX_STATE, TRX_CMD_RX_ON);              // Put the radio back to ON state 

    for(uint8_t i=0; i<PMP_MEASUREMENT_SIZE; i++){
        phase[i] = phases[i];
    }

    //printf("PMP routine duration %d\n", RTIMER_NOW() - start);
    return 1;
}








/*------------------------------------------------------------------------------------------*/
/* Functions for testing purposes */
/*------------------------------------------------------------------------------------------*/

uint16_t FREQ = 2344;
/*------------------------------------------------------------------------------------------*/
void
rf233_test_CW(void){

    vsnTime_delayS(2);

    uint16_t freq = FREQ;
    uint8_t offset = 1;

    // Configure the radio for CW
    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);
    
    bitWrite(SR_TOM_EN, 0);                     // Enable PMU unit
    bitWrite(SR_PMU_EN, 1);

    bitWrite(SR_TX_PWR, 0xD);                   // Set output power to max (0x00)
    bitWrite(SR_TX_AUTO_CRC_ON, 0);             // Disable TX_AUTO_CRC_ON

    bitWrite(SR_MOD_SEL, 1);		            // Manual control of modulation data
	bitWrite(SR_MOD, 0);			            // Continuous 0 chips

    bitWrite(SR_TX_RX, 1);				        // TX PLL frequency = 1, RX = 0, default = 0
	bitWrite(SR_TX_RX_SEL, 1);		            // Manual control of PLL frequency mode

    // Set the frequency
    if (freq < 2434){
        regWrite(RG_CC_CTRL_1, 0x08);
        freq -= 2306;
    }
    else if (freq < 2528){
        regWrite(RG_CC_CTRL_1, 0x09);
        freq -= 2434;
    }
    freq = freq << 1;
    if(offset){
        freq += 1;
    }
    regWrite(RG_CC_CTRL_0, (uint8_t) freq);


    // Go to PLL_ON and start transmitting
    LOG_INFO("CW on frequency: CC_NUM = %d \n", freq);

    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_PLL_ON);
    regWrite(RG_TRX_STATE, TRX_CMD_TX_START);
    while(1){
        if(bitRead(SR_TRX_STATUS) != TRX_STATUS_BUSY_TX){
            regWrite(RG_TRX_STATE, TRX_CMD_TX_START);
        }
    }
}
/*------------------------------------------------------------------------------------------*/
void
rf233_test_PMU(void){
    uint16_t freq = FREQ;
    uint8_t offset = 0;

    vsnTime_delayS(1);

    // Configure the radio for PMU 
    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_TRX_OFF);

    bitWrite(SR_TOM_EN, 0);                         // Enable PMU unit
    bitWrite(SR_PMU_EN, 1);
    bitWrite(SR_RX_PDT_DIS, 1);                     // Disable RX path
    
    bitWrite(SR_PMU_IF_INVERSE, 1);                 // Set IF inverse
    bitWrite(SR_TX_RX, 0);				            // TX PLL frequency = 1, RX PLL freq = 0
	bitWrite(SR_TX_RX_SEL, 1);	                    // Manual control of PLL frequency mode

    // Set frequency
    if (freq < 2434){
        regWrite(RG_CC_CTRL_1, 0x08);
        freq -= 2306;
    }
    else if (freq < 2528){
        regWrite(RG_CC_CTRL_1, 0x09);
        freq -= 2434;
    }
    freq = freq << 1;
    if(offset){
        freq += 1;
    }
    regWrite(RG_CC_CTRL_0, (uint8_t) freq);


    // Go to RX_ON and start measuring
    LOG_INFO("PMU on frequency: CC_NUM = %d \n", freq);
    regWrite(RG_TRX_STATE, TRX_CMD_FORCE_PLL_ON);
    regWrite(RG_TRX_STATE, TRX_CMD_RX_ON);

    uint8_t phase[100] = {0};
    uint8_t loop = 0;
    while(1){
        for(uint8_t i=0; i<100; i++){
            phase[i] = regRead(RG_PHY_PMU_VALUE);
        }
        loop++;
        printf("phase_%d = [", loop);
        for(uint8_t i=0; i<100; i++){
            printf("%d , ",phase[i]);
            phase[i] = 0;
        }
        printf("]\n");

        vsnTime_delayUS(100);
    }

}

#endif