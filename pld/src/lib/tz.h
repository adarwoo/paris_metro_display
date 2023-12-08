#ifndef tz_h_HAS_ALREADY_BEEN_INCLUDED
#define tz_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup service
 * @{
 * @addtogroup timezone
 * Work out local CET time and daylight saving
 * @{
 * @file
 * Timezone helper API
 * @author software@arreckx.com
 *****************************************************************************
 * Relies on the ASF calendar service
 */ 

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "calendar.h"

/************************************************************************/
/* Public API                                                           */
/************************************************************************/

/**
 * Calendar structure to hold a date.
 * Compatible with the asf calendar date
 */
typedef struct calendar_date tz_datetime_t;

/** Year after which this device is used */
#define TZ_SERVICE_YEAR 2016

/** Number of seconds in a day */
#define TZ_SECS_PER_DAY 86400L

/** Reference EPOCH year */
#define TZ_EPOCH_YEAR 1970

/** Number of seconds since epoch */
#define TZ_SERVICE_EPOCH ( \
   (TZ_SERVICE_YEAR-TZ_EPOCH_YEAR) * TZ_SECS_PER_DAY * 365 + \
   ((TZ_SERVICE_YEAR-1969)/4)   * TZ_SECS_PER_DAY       - \
   ((TZ_SERVICE_YEAR-1901)/100) * TZ_SECS_PER_DAY       + \
   ((TZ_SERVICE_YEAR-TZ_EPOCH_YEAR+299)/400) * TZ_SECS_PER_DAY \
)

/** Adjust the time for CET timezone */
uint32_t tz_convert_to_cet( uint32_t epochIn );

/** Check the time is valid */
static inline bool tz_time_is_valid(uint32_t epochIn)
   { return epochIn > TZ_SERVICE_EPOCH; }
      
/** Returns the time now or a null pointer if the time is not valid */      
tz_datetime_t *tz_now( tz_datetime_t *pDate );

#ifdef __cplusplus
}
#endif

/**@} timezone */
/**@} service */
#endif /* tz_h_HAS_ALREADY_BEEN_INCLUDED */
