/* Project configuration */
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

// All logs to LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAIN                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_AT86RF215                   LOG_LEVEL_INFO
#define TSCH_LOG_CONF_PER_SLOT                     (1)

#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE          (uint8_t[]){11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26}
//#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE          (uint8_t[]){11}

#define AT86RF2XX_CONF_PACKET_STATS                     (1)
#define AT86RF2XX_CONF_DRIVER_STATS                     (1)

#endif /* PROJECT_CONF_H_ */