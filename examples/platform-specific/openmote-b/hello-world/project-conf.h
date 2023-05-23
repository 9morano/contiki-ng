#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define LOG_CONF_LEVEL_MAIN                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_WARN



/* Configure OpenMote B*/
#define WATCHDOG_CONF_ENABLE                        (0)
#define OPENMOTEB_CONF_USE_ATMEL_RADIO              (1)

/* Configure Atmel Radio */
#define LOG_CONF_LEVEL_AT86RF215                    LOG_LEVEL_INFO


#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE          (uint8_t[]){11}
#define TSCH_LOG_CONF_PER_SLOT                      (1)


#endif