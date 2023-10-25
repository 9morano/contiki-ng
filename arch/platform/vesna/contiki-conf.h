#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

#include <stdint.h>
#include <string.h>

#define CCIF
#define CLIF

#ifdef PROJECT_CONF_PATH
#include PROJECT_CONF_PATH
#endif

/*---------------------------------------------------------------------------*/
/* Platfrom specific typedefs */
typedef uint64_t clock_time_t;
typedef uint32_t uip_stats_t;

/*---------------------------------------------------------------------------*/
#define CLOCK_CONF_SECOND	(1000)

/*---------------------------------------------------------------------------*/
/* Platform specific timer bit-size - STM32F103 has 16-bit counter */
#define RTIMER_CONF_CLOCK_SIZE  (2)
#define RTIMER_CONF_GUARD_TIME  (7)

/*---------------------------------------------------------------------------*/
/* Does platform support button */
#define PLATFORM_SUPPORTS_BUTTON_HAL  (1)

/*---------------------------------------------------------------------------*/
/* Enable/disable stack check. Linker script doesn't define proper 
 * pointers to enable this feature */
#define STACK_CHECK_CONF_ENABLED	(0)

/*---------------------------------------------------------------------------*/
/* board.h assumes that basic configuration is done */
#include "board.h"

/*---------------------------------------------------------------------------*/
/* Include CPU-related configuration */

#endif
