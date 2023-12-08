/**
 * @addtogroup driver
 * @{
 * @addtogroup key
 * @{
 *****************************************************************************
 * @details Manages a single key or push button.
 * To use this API, first initialise it calling keyInit. A couple of callbacks
 *  are required. These are called when a key press is detected.
 * Also, the function keyScan must be called periodically, ideally every 20ms
 *  (50Hz) or thereabout.
 * The parameters #KEY_DEBOUNCE_CYCLES and #KEY_LONG_CYCLES can be adjusted
 *  in config.h to modify the responsivity of the keypad. They are set for a
 *  50Hz sampling rate at 3 and 40 respectively.
 * The function keyScan can be called within an interrupt, but the callback
 *  method would also be called with the same interrupt.
 *****************************************************************************
 * @file
 * Key API implementation
 * @author software@arreckx.com
 * @internal
 */

#include <asf.h>

#include "driver/key.h"
#include "lib/reactor.h"

/**
 * Set the default values.
 * For KEY_DEBOUNCE_CYCLES and KEY_LONG_CYCLES, we assume a 50Hz sampling rate
 * @def KEY_SAMPLING_RATE
 * Rate at which to sample the key. Defaults to #KEY_PREFERED_SCANNING_RATE
 * @def KEY_DEBOUNCE_CYCLES
 * Number of steady cycle to observe to redeem the press valid. Defaults to 3.
 * @def KEY_LONG_CYCLES
 * Number of cycles to observe to validate a long key press. Defaults to 40.
 **/
#if !defined KEY_SAMPLING_RATE
   #define KEY_SAMPLING_RATE KEY_PREFERED_SCANNING_RATE
#endif

#if !defined KEY_DEBOUNCE_CYCLES
   #define KEY_DEBOUNCE_CYCLES 3
#endif

#if !defined KEY_LONG_CYCLES
   #define KEY_LONG_CYCLES 40
#endif

/** For a very long push cycle */
#define _VERY_LONG_CYCLES (KEY_LONG_CYCLES*3)

#if !defined KEY_PIN
   #error "The key API requires the KEY_PIN resource to be defined"
#endif

/** Mask to check for the short push flag in the dispatch flags */
#define _KEY_SHORT_DISPATCH_FLAG_MASK (1<<1)

/** Mask to check for the long push flag in the dispatch flags */
#define _KEY_LONG_DISPATCH_FLAG_MASK (1<<2)

/** Mask to check for the very long push flag in the dispatch flags */
#define _KEY_VERY_LONG_DISPATCH_FLAG_MASK (1<<3)

// Reassign macro defined constants in typed variables

/** Number of samples to debounce a key */
static const uint8_t _key_debounce_cycles = KEY_DEBOUNCE_CYCLES;

/** Number of sampling cycles for a long push to be detected */
static const uint8_t _key_long_cycles = KEY_LONG_CYCLES;

/** Defines different stages of the keypad state machine */
typedef enum
{
   /** No key activity */
   keyIdle_e = 0,
   /** Single, breif key press */
   keySingle_e,
   /** Long key press */
   keyLong_e,
   /** Very long press */
   keyVeryLong_e,
} keyState_t;


/** Keep the current state of the key */
static keyState_t state;

/** Count how many cycles the key was down */
static uint8_t consecutiveDowns;

/** Function to call when a quick push is detected */
static keyPushedCallback_t keyShortPushCallBack;

/** Function to call when a long push is detected */
static keyPushedCallback_t keyLongPushCallBack;

/** Function to call when a long push is detected */
static keyPushedCallback_t keyVeryLongPushCallBack;

/** Flag to indicate which callback to call */
static volatile uint8_t _key_dispatch_flags = 0;

/** Reactor processing handle to dispatch on key pressed */
static volatile reactor_handle_t _key_reactor_handle = 0;


/**
 * Dispatch to the callbacks if a key was pressed
 */
static void _key_dispatch( void )
{
   // Fast first test (pretty much every time)
   if ( _key_dispatch_flags )
   {
      if ( _key_dispatch_flags & _KEY_SHORT_DISPATCH_FLAG_MASK )
      {
         if ( keyShortPushCallBack )
         {
            keyShortPushCallBack();
         }
      }
   
      if ( _key_dispatch_flags & _KEY_LONG_DISPATCH_FLAG_MASK )
      {
         if ( keyLongPushCallBack )
         {
            keyLongPushCallBack();
         }
      }
      
      if ( _key_dispatch_flags & _KEY_VERY_LONG_DISPATCH_FLAG_MASK )
      {
         if ( keyVeryLongPushCallBack )
         {
            keyVeryLongPushCallBack();
         }
      }
      
      // Reset the flags always. 
      // Type if atomic and both flags at once is not possible
      _key_dispatch_flags = 0;
   }      
}   

/**
 * Scan the keypad for changes.
 * It is up to the application to call this method periodically
 *  every 20ms or thereabout.
 *
 * Called within a timer interrupt context
 */
static void keyScan(void)
{
   // Scan the key
   if ( ioport_pin_is_low(KEY_PIN) )
   {
      // Key is down
      switch ( state )
      {
      case keyIdle_e:
         if ( ++consecutiveDowns > _key_debounce_cycles )
         {
            state = keySingle_e;
         }
         break;
      case keySingle_e:
         if ( ++consecutiveDowns > _key_long_cycles )
         {
            // Change state and immediately notify the state machine of a long key push
            state = keyLong_e;
            _key_dispatch_flags |= _KEY_LONG_DISPATCH_FLAG_MASK;
         }
         break;
      case keyLong_e:
         // The long is called - but the very long usually kills the app
         if ( ++consecutiveDowns > _VERY_LONG_CYCLES )
         {
            state = keyVeryLong_e;
            _key_dispatch_flags |= _KEY_VERY_LONG_DISPATCH_FLAG_MASK;
         }
         break;
      case keyVeryLong_e:
         // Stay there until the key is released
         break;
      }
   }
   else
   {
      switch ( state )
      {
      case keyIdle_e:
         consecutiveDowns = 0;
         break;
      case keySingle_e:
         // If the switch was pushed more than the debounce (but less than a long push),
         //  validate the push
         if ( consecutiveDowns > _key_debounce_cycles )
         {
            // Change state and immediately notify the state machine of a long key push
            state = keyIdle_e;
            _key_dispatch_flags |= _KEY_SHORT_DISPATCH_FLAG_MASK;
         }
         consecutiveDowns = 0;
         break;
      case keyLong_e:
      case keyVeryLong_e:
         state = keyIdle_e;
         consecutiveDowns = 0;
         break;
      }
   }
   
   // Tell the reactor that processing is required
   if ( _key_dispatch_flags )
   {
      reactor_notify(_key_reactor_handle);
   }
}


/**
 * Initialise the keypad and its state machine
 */
void key_init(
   keyPushedCallback_t shortPushCallBack, 
   keyPushedCallback_t longPushCallBack,
   keyPushedCallback_t veryLongPushCallBack )
{
   // Store the callbacks
   keyShortPushCallBack = shortPushCallBack;
   keyLongPushCallBack = longPushCallBack;
   keyVeryLongPushCallBack = veryLongPushCallBack;

   // Activate the pull-up on the port pin
   ioport_set_pin_mode( KEY_PIN, IOPORT_MODE_PULLUP);

   // Configure as input
   ioport_set_pin_dir( KEY_PIN, IOPORT_DIR_INPUT);

   // Reset key state
   state = keyIdle_e;
   consecutiveDowns = 0;
   
   // Arm the timer
   // Enable the timer
   tc_enable( &KEY_TIMER_TC );

   // Simple timer compare mode required
   tc_set_wgm( &KEY_TIMER_TC, TC_WG_NORMAL );

   // The timer service interrupts run at low level to allow
   //  other interrupts to take precedence
   tc_set_overflow_interrupt_level( &KEY_TIMER_TC, TC_INT_LVL_MED );

   // Set the callback
   tc_set_overflow_interrupt_callback( &KEY_TIMER_TC, &keyScan );

   // Aiming for 20ms - that's 50Hz clock
   // That's 640,000 divider overall
   // 2^10 * 625
   
   // Set the top to 
   tc_write_period( &KEY_TIMER_TC, 625 );

   // Source is main clock (assuming 32MHz) divided by 256
   // Gives a overall clock as 125kHz
   // This effectively starts the timer ticking
   tc_write_clock_source( &KEY_TIMER_TC, TC_CLKSEL_DIV1024_gc );
   
   // Regsiter with the reactor for power saving
   _key_reactor_handle = reactor_register(&_key_dispatch);
}

/**@}*/
/**@} ------------------------  End of file  ---------------------------- */
