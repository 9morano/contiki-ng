#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define LOG_CONF_LEVEL_MAIN                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_WARN
#define TSCH_LOG_CONF_PER_SLOT                     (1)


/* Configure OpenMote B*/
#define WATCHDOG_CONF_ENABLE                        (0)
#define OPENMOTEB_CONF_USE_ATMEL_RADIO              (1)

/* Configure Atmel Radio */
#define LOG_CONF_LEVEL_AT86RF215                    LOG_LEVEL_INFO

/* Enable PMP during TSCH routine - development */
#define TSCH_CONF_WITH_PMP                          (1)  

/* Select desired channels */
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE          (uint8_t[]){26}
//#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE        (uint8_t[]){11, 15, 20, 26}
//#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE        (uint8_t[]){11, 13, 15, 17, 19, 21, 23, 25}
//#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE        (uint8_t[]){11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26}

#endif