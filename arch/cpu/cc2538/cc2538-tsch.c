/*
 *  IEEE 802.15.4 TSCH timeslot timings for CC2538 radio and At86RF231 radio chip. 
 *  Timeslot length is 15ms.
 * 
 */

#include "contiki.h"
#include "net/mac/tsch/tsch.h"



const tsch_timeslot_timing_usec tsch_timing_cc2538_15ms = {
    1800,   // CCAOffset
    128,    // CCA

    3000,   // TxOffset
    (3000 - (TSCH_CONF_RX_WAIT / 2)), // RxOffset
    3000,   // RxAckDelay
    3800,   // TxAckDelay
    TSCH_CONF_RX_WAIT, // RxWait (PGT)
    2000,   // AckWait (AGT)
    2072,   // RxTx (Not used)
    2400,   // MaxAck (TxAck)
    4256,   // MaxTx (TxPacket)
    15000,  // TimeslotLength
};