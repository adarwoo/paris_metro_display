#ifndef key_HAS_ALREADY_BEEN_INCLUDED__
#define key_HAS_ALREADY_BEEN_INCLUDED__
/**
 * @file
 * Key API header
 * @author software@arreckx.com
 * @addtogroup driver
 * @{
 * @addtogroup key
 * @{
 *****************************************************************************
 * Defines the single key API.
 * - This API uses the reactor for dispatch.
 * - The key API must be initialised by calling keyInit.
 * - The KEY_PORT and KEY_PIN resource must be defined in config.h.
 *****************************************************************************
 */

/** Preferred key sampling rate in Hz*/
#define KEY_PREFERED_SCANNING_RATE 50

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * Callback type to be passed in the init method called 
 *  on a short or long push
 */
typedef void (*keyPushedCallback_t)(void);

/** Ready the API and hook into the application */
void key_init(
   keyPushedCallback_t shortPushCb, 
   keyPushedCallback_t longPushCb,
   keyPushedCallback_t vlongPushCb );

/** Called handlers from the reactor loop */   
void key_dispatch(void);

#ifdef __cplusplus
}
#endif

/**@}*//**@}*/
#endif   /* ndef key_HAS_ALREADY_BEEN_INCLUDED__ */