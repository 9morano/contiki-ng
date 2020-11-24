/* Project configuration */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

// Increase log levels
#define LOG_CONF_LEVEL_IPV6                         LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_RPL                          LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_6LOWPAN                      LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_TCPIP                        LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAC                          LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_FRAMER                       LOG_LEVEL_WARN
#define TSCH_LOG_CONF_PER_SLOT                      (1)

// Testing only on channel 26 (faster joining)
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE          (uint8_t[]){ 26 }


#endif /* PROJECT_CONF_H_ */