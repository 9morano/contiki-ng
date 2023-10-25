#ifndef VESNA_CONF_H_
#define VESNA_CONF_H_

#include "board.h"

/* Enable/disable hardware watchdog */
#ifndef VESNA_CONF_WATCHDOG_ENABLED
#define VESNA_WATCHDOG_ENABLED      (0)
#else
#define VESNA_WATCHDOG_ENABLED      (VESNA_CONF_WATCHDOG_ENABLED)
#endif

/* Enable/disable UART */
#ifndef VESNA_CONF_UART1_ENABLED
#define VESNA_UART1_ENABLED         (1)
#else
#define VESNA_UART1_ENABLED         (VESNA_CONF_UART1_ENABLED)
#endif

/* Enable/disable SLIP */
#ifndef VESNA_CONF_SLIP_ENABLED
#define VESNA_SLIP_ENABLED          (0)
#else
#define VESNA_SLIP_ENABLED          (VESNA_CONF_SLIP_ENABLED)
#endif

/* Set default baud rate for UART1 */
#ifndef VESNA_CONF_UART1_BAUDRATE
#define VESNA_UART1_BAUDRATE        (115200)
#else
#define VESNA_UART1_BAUDRATE        (VESNA_CONF_UART1_BAUDRATE)
#endif

/* Use external clock source (used at SNR and SNE_ATASW) */
#ifndef VESNA_CONF_USE_EXTERNAL_CLOCK
#define VESNA_USE_EXTERNAL_CLOCK    (0)
#else
#define VESNA_USE_EXTERNAL_CLOCK    (VESNA_CONF_USE_EXTERNAL_CLOCK)
#endif

/* Rtimer can use external clock source (used at SNE boards) */
#ifndef VESNA_CONF_RTIMER_USE_EXTERNAL_SOURCE
#define VESNA_RTIMER_USE_EXTERNAL_SOURCE    (0)
#else
#define VESNA_RTIMER_USE_EXTERNAL_SOURCE    (VESNA_CONF_RTIMER_USE_EXTERNAL_SOURCE)
#endif

#endif