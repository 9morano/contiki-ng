/** 
 * \file
 *         Header file for the rf233_pmu.c
 * \brief
 *         The realization of phase measurement with AT68$F233 radio.
 */


#ifndef RF233_PMU_H_
#define RF233_PMU_H_

// Include GPIO debug pins
#define PMP_GPIO_DEBUG                  (1)

// Guard time - wait for other to prepare
#define PMP_GUARD_TIME                  (40)
#define PMP_TRANSITION_TIME             (30)

// The output power of CW signal
#define PMP_CW_POWER                    (0x00)

// The starting frequency of Phase Measurement
#define PMP_FIRST_CHANNEL               (2400)

// Number of sampled phases in one PMP
#define PMP_MEASUREMENT_SIZE            (15)

// Golomb ruler
// Thanks to: O. Oshiga, A. Ghods, S. Severi, and G. Abreu, “Efficient Slope Sampling Ranging and Trilateration Techniques for Wireless Localization”
uint8_t  golomb_ruler[] = {1, 12, 15, 16, 25, 46, 62, 85, 104, 121, 126, 133, 153, 159, 161};
uint16_t golomb_freq[] = {2400, 2406, 2407, 2408, 2412, 2423, 2431, 2442, 2452, 2460, 2463, 2466, 2476, 2479, 2480};
uint8_t  golomb_offset[] = {1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1};


// Struct to store the measurements (obsolete)
typedef struct {
    uint8_t channel;
    uint8_t sqn;
    uint8_t data[PMP_MEASUREMENT_SIZE];
} pmuData_t;


// Struct of important registers
struct {
    uint8_t trx_ctrl_0;
    uint8_t trx_ctrl_1;
    uint8_t trx_ctrl_2;
    uint8_t phy_tx_pwr;
    uint8_t xah_ctrl_0;
    uint8_t xah_ctrl_1;
    uint8_t cc_ctrl_0;
    uint8_t cc_ctrl_1;
    uint8_t csma_seed_0;
    uint8_t csma_be;
    uint8_t tst_sdm;
    uint8_t tst_agc;
    uint8_t rx_syn;
} bkp_register;

// Copied from rf2xx_hal
#define DEFAULT_IRQ_MASK    (IRQ2_RX_START | IRQ3_TRX_END | IRQ4_CCA_ED_DONE | IRQ5_AMI | IRQ6_TRX_UR)



/**
 * \brief      Backup important registers.
 *
 *     This function stores the values of the registers that
 *     are used and therefore changed in phase measurement 
 *     process. The data is stored in struct bkp_register. 
 */
void rf233_register_backup(void);

/**
 * \brief      Restore important registers.
 *
 *     This function restores the values of the registers 
 *     that were used and in phase measurement process. 
 */
void rf233_register_restore(void);

/**
 * \brief      Prepare the radio for PMP.
 *
 *     This function configures the radio for phase measurement
 *     process. It enables the PMU of a radio and configures
 *     the modulation source for CW generation.
 */
void rf233_prepare_for_PMP(void);

/**
 * \brief      Transmit a CW signal.
 * \param duration The duration of a CW signal.
 *
 *     This function generates continuous wave (CW) for a
 *     defined duration (in us).
 */
void rf233_transmit(uint32_t duration);

/**
 * \brief      Get the measured phase sample
 * \param data A pointer to measured data.
 *
 *     This function obtains the measured phase sample stored
 *     in the radio's buffer. 
 */
void rf233_read_phase(uint8_t *data);

/**
 * \brief      Set the desired frequency (in steps of 0.5MHz)
 * \param f    Desired frequency (in range 2434 to 2528)
 * \param o    Additional offset of 0.5 MHz
 *
 *     This function sets the PLL of a radio to desired freq.
 */
void rf233_set_frequency(uint16_t f, uint8_t o);

/**
 * \brief      The phase measurement process (PMP).
 * \param role 0 = INIT, 1 = REF
 * \param channel The current TSCH channel
 * \param phase A pointer to the measured data 
 * \return     1 if success, 0 otherwise
 *
 *     This function implements the phase measurement process.
 *     It first prepares the radio (stores all of the registers 
 *     and configures it for CW and PMU). Depending on the role
 *     it then transmits the CW / measures the phase angle of
 *     the comm channel. The process is repeated PMP_MEASUREMENT_SIZE.
 *     The channel param is left for future upgrades.
 */
/*--------------------------------------------------------------------*/
int rf233_phase_measurement_process(uint8_t role, uint8_t channel, uint8_t *phase);




/*--------------------------------------------------------------------*/
void rf233_test_CW(void);
void rf233_test_PMU(void);

#endif