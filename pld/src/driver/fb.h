#ifndef fb_HAS_ALREADY_BEEN_INCLUDED
#define fb_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup driver
 * @{
 * @addtogroup framebuffer
 * @{
 *****************************************************************************
 * API and management of the LED frame buffer.
 * 
 * The frame buffer API has two functions:
 * 1. It actually manages the LED on the board through the careful use of
 *     DMA, SPI and timers to operate almost entirely in the background
 * 2. It provides an API which allow controlling the LEDs on the display through
 *     simple functions.
 *
 * The user should control the LED only through the public API of the frame buffer.
 * To avoid glitches, the API allow the user to make changes in a temporary buffer
 *  before committing all the changes at once.
 * This allow for example to clear the frame buffer, make changes and commit without
 *  having the screen flicker at all during the clear.
 * By default, the API operates in a temporary frame buffer.
 * Example:
 * @code
 * #include "driver/fb.h"
 * #include "core/topo.h" // For stations names
 * // Create a local frame buffer
 * static fb_mem_t fb = {0}; // Reset to all off the first time
 * // Clear some
 * fb_turn_off( POISSY );
 * // Turn full on other
 * fb_turn_on( AUBER );
 * // Increase the luminosity level
 * fb_led_t l=fb_get_composite(GARE_DE_LYON);
 * ++l.level;
 * fb_set_composite(GARE_DE_LYON, l);
 * @endcode
 * @file
 * Definition of the frame buffer for the system LEDs
 * @author gax
 */

#include <stdint.h>
#include <string.h> // memcpy

#include "config/conf_board.h"

#define LED_OFF          0x0 ///< Led if turned off. Nothing else matters.
#define LED_ON           0xF ///< Led is steadily on
#define LED_FLASH_SLOW   0x8 ///< Led is flashing every second
#define LED_FLASH_MEDIUM 0x4 ///< Led is flashing twice a second
#define LED_FLASH_FAST   0x2 ///< Led is flashing at 4 Hz
#define LED_FLASH_VFAST  0x1 ///< Led is flashing at 8Hz

#define LED_LEVEL_FULL   0xf ///< Led level is maxed out
#define LED_LEVEL_MED    0x8 ///< Led is at 50%
#define LED_LEVEL_LOW    0x1 ///< Led is at 12.5%

#ifdef __cplusplus
extern "C" {
#endif

/** Index of a single LED */
typedef uint8_t fb_index_t;

/** 
 * Set the state for a single LED
 * Each LED is a single byte, therefore atomic
 */
typedef struct _fb_led_s 
{
   uint8_t status : 4, ///< Status (one of #LED_ON, etc..)
   level  : 4;         ///< Luminosity level 0 to 15 or a define  such as #LED_LEVEL_MED
} fb_led_t;

/**
 * Framebuffer containing the state of each LED in the matrix
 * The framebuffer is ordered to make reading it faster than writing.
 */
typedef fb_led_t fb_mem_t[FB_BITS_PER_DRIVER * FB_NUMBER_OF_DRIVERS];

/** State of all the leds in the system */
extern fb_mem_t fb_current;

/** Current working copy of the buffer. This is to prevent glitches. */
extern fb_mem_t *fb_live;

/************************************************************************/
/* API                                                                  */
/************************************************************************/

/** Initialise the framebuffer API */
void fb_init(void);

/************************************************************************/
/* Inline implementations                                               */
/************************************************************************/

/** Reset the working copy of the frame buffer */
inline void fb_clear(void)
   { memset( (void *)(*fb_live), 0, sizeof(fb_current)); }

/** 
  * Set the working copy of the frame buffer to use from now on.
  * All subsequent use of the following functions:
  * - #fb_set_composite
  * - #fb_set
  * - #fb_get_composite
  * - #fb_turn_on
  * - #fb_turn_off
  *
  * will be making changes to this frame buffer.
  *
  * @param to A pointer to a valid #fb_mem_t pointer.
  */
inline void fb_use( fb_mem_t *to )
   { fb_live = to; }

/**
 * Copy the content of the working frame buffer into the active frame buffer
 * Note : No locking of interrupts here. It's OK if the frame buffer is not
 *        fully copied. Each group of LED will be atomically.
 */
inline void fb_commit(void)
   { memcpy( (void *)fb_current, (void *)(*fb_live), sizeof(fb_current) ); }

/** Set a LED using a composite value */
inline void fb_set_composite(fb_index_t index, fb_led_t state)
   { (*fb_live)[index] = state; }

/** Set an LED */
inline void fb_set(fb_index_t index, uint8_t status, uint8_t level )
   { fb_led_t s = {status, level}; (*fb_live)[index] = s; }

/** Get the composite value */
inline fb_led_t fb_get_composite(const fb_index_t index)
   { return (*fb_live)[index]; }

/** Turn full on an LED */
inline void fb_turn_on(fb_index_t index)
   { fb_led_t s = {LED_ON, LED_LEVEL_FULL}; (*fb_live)[index] = s; }

/** Turn off an LED */
inline void fb_turn_off(fb_index_t index)
   { fb_set(index, 0, 0); }

#ifdef __cplusplus
}
#endif
   
/** @} */
/** @} */
#endif /* fb_HAS_ALREADY_BEEN_INCLUDED */
