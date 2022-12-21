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
#define LOG_CONF_LEVEL_RF2XX                       LOG_LEVEL_INFO
#define TSCH_LOG_CONF_PER_SLOT                     (1)

// Defines for app
#define UART1_CONF_BAUDRATE                        (460800)
#define WATCHDOG_CONF_ENABLED                      (0)

#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE          (uint8_t[]){11}

// Enable the phase measurement process
#define TSCH_CONF_WITH_PMP                          (1)

#endif /* PROJECT_CONF_H_ */