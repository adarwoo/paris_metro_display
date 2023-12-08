/**
 * @file
 * <b>Application main entry point</b>.
 * @author software@arreckx.com
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <asf.h>
#include <sio2host.h>

// Local includes
#include "driver/fb.h"
#include "driver/key.h"

#include "lib/alert.h"
#include "lib/timer.h"
#include "lib/reactor.h"

#include "core/sequencer.h"
#include "core/measurements.h"
#include "core/gps_manager.h"

/**
 * Main entry point called once the build-in initialization is complete
 * This function must not return ever.
 *
 * This display is a FSM with all transitions managed by a timer callback.
 * The main task is used to manage the GPS data and process all the timer callback
 *  very much like a reactor loop.
 * The watchdog is activated to make sure the reactor look is alive.
 */
int main(void)
{
   // Activate the watchdog early (should be activated by the hardware fuse)
   // The reactor will kick the watchdog for every dispatch done
   wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_125CLK);
#ifndef DEBUG
   wdt_enable();
#endif

   //
   // Initialize all core services and application dedicated services
   // 
   alert_init();       // Allow alerts
   board_init();       // Clocks, I/O, etc.
   reactor_init();     // Prepare the reactor
   rtc_init();         // Ready the RTC and reset the clock
   sio2host_init();    // Initialize the serial I/O library
   timer_init();       // Ready the timer API
   fb_init();          // Ready the frame buffer API
   measurement_init(); // Ready the systems measurements (lum and temp)
   key_init(           // Ready the key pad API
      &sequencer_switch_short, 
      &sequencer_switch_long, 
      &reset_do_soft_reset);
   gps_manager_init(); // Read the GPS manager
   
   // Start the sequencer to start the led displays
   sequencer_start();

   // All is set, let the reactor run... forever
   reactor_run();
}

/** @} ------------------------  End of file  ---------------------------- */