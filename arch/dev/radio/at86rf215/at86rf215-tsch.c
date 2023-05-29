#include "contiki.h"
#include "net/mac/tsch/tsch.h"


const tsch_timeslot_timing_usec tsch_timeslot_timing_at86rf215_10000us_250kbps = {
   1800, // CCAOffset 
    128, // CCA 
   2120, // TxOffset 
  (2120 - (TSCH_CONF_RX_WAIT / 2)), // RxOffset 
   800, // RxAckDelay 
   1000, // TxAckDelay 
  TSCH_CONF_RX_WAIT, // RxWait
    400, // AckWait 
    192, // RxTx 
   2400, // MaxAck 
   4256, // MaxTx 
  10000, // TimeslotLength 
};

const tsch_timeslot_timing_usec tsch_timeslot_timing_at86rf215_12000us_250kbps_with_pmp = {
   1800, // CCAOffset 
    128, // CCA 
   2120, // TxOffset 
  (2120 - (TSCH_CONF_RX_WAIT / 2)), // RxOffset 
   800, // RxAckDelay 
   1000, // TxAckDelay 
  TSCH_CONF_RX_WAIT, // RxWait
    400, // AckWait 
    192, // RxTx 
   2400, // MaxAck 
   4256, // MaxTx 
  12000, // TimeslotLength 
};