/**
 * @file
 * Alert API implementation
 * @author gax
 * @internal
 * @addtogroup service
 * @{
 * @addtogroup alert
 * @{
 */
#include <asf.h>
#include <assert.h>

#include "alert.h"

#ifdef ALERT_RECORD
#   ifndef ALERT_RECORD_OFFSET
#      define ALERT_RECORD_OFFSET 0
#   endif
#endif

/**
 * We need to initialize the alert API
 * For LED notification, we need to set the LED direction.
 * For EEPROM storage, we need to ready some variables
 */
void alert_init( void )
{
#ifdef ALERT_OUTPUT_PIN
   ioport_set_pin_dir( ALERT_OUTPUT_PIN, IOPORT_DIR_OUTPUT );

   // The mode can be used to change the pin polarity
   #ifdef ALERT_OUTPUT_MODE
   {
      ioport_set_pin_mode( ALERT_OUTPUT_PIN, ALERT_OUTPUT_PIN);
   }
   #endif
#endif
}

/**
 * General way on alerting of an assertion using whatever mechanism
 *  is available on the target.
 *
 * This function is best called through the macros alert an alert_and_stop.
 *
 * @param doAbort If true, the abort method is called to halt the cpu
 * @param line Line where to alert took place.
 * @param file Name of the file where the exception occurred.
 */
void alert_record( bool doAbort, int line, const char *file )
{
   // Output to stdout?
   #ifdef ALERT_TO_STDOUT
   {
      printf_P("ALERT: %s, line %d\n", file, line);
   }
   #endif

   // Stop or not
   if ( doAbort )
   {
      #ifdef DEBUG
         // Stop the watchdog for debug only so the program will hang rather
         //  than reset
         wdt_disable();
      #endif
      // Lock forever. Breaking the execution will end up here
      //  (or an interrupt), and the trace will point to the
      //  culprit.
      // The watchdog will then reset.
      for (;;)
      {
         // Dump to the debug pin
         #ifdef ALERT_OUTPUT_PIN
         {
            // Transmit as X_X___X_X___XXX___X_X which can be read easily off any scope
            uint_fast16_t l = line;
            uint_fast8_t i, j;
            
            // Transmit the 12-bits as 4 bits for easy read
            for (i=0; i<12; ++i)
            {
               uint8_t tx = (l & 0x800) ? 0b011100 : 0b010100;
               
               for (j=0; j<6; ++j)
               {
                  ioport_set_pin_level( ALERT_OUTPUT_PIN, (bool) (tx & 1) );
                  tx >>= 1;
               }
               
               l <<= 1;
            }
            
            // Transmit 24*4 empty bits for the scope to resync easy
            for (i=0; i<24*4; ++i)
            {
               ioport_set_pin_level( ALERT_OUTPUT_PIN, false );
            }
         }
         #endif
         continue;
      }
   }
}

 /**@}*/
 /**@} ---------------------------  End of file  --------------------------- */