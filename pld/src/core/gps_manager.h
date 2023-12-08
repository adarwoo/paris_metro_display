#ifndef gps_manager_h_HAS_ALREADY_BEEN_INCLUDED
#define gps_manager_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @file
 * GPS service API
 * @addtogroup service
 * @{
 * @addtogroup gps
 * @{
 * API for managing an instance of gps.
 * The data gathered is used to update the system time.
 * @author gax
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Init the API */
void gps_manager_init(void);

/** Call to check if a new character needs processing */
void gps_manager_update(void);

/** Configure the GPS module */
void gps_configure(void);

/** Wake up the module */
void gps_activate(void);

/** Put the module in stand-by */
void gps_standby(void);

#ifdef __cplusplus
}
#endif

/**@}*/
/**@}*/
#endif /* ndef gps_manager_h_HAS_ALREADY_BEEN_INCLUDED */