/**
 * @addtogroup driver
 * @{
 * @addtogroup framebuffer
 * @{
 * @file
 * LED Frame buffer management
 */
#include <asf.h>
#include <string.h>

#include "lib/debug.h"
#include "lib/timer.h"
#include "driver/fb.h"
#include "core/measurements.h"


/** 
 * Local defines
 */

/** Upper limit for the luminosity duty cycle */
#define _UPPER_LUM_COUNT 32



/** 
 * @def _REFRESH_RATE
 * Rate at which the LEDs are refreshed in Hz.
 * The non optimization of debug starves the CPU of time
 * @def _REFRESH_RATE_POW2
 * Number of bits to shift the blinking counter to take into
 *  account the refresh rate.
 */
#ifdef NDEBUG
   #define _REFRESH_RATE 256
#else
   #define _REFRESH_RATE 128
   #define _REFRESH_RATE_POW2 5
#endif

/** Rate at which to reload the drivers */
#define _CYCLE_RATE (_UPPER_LUM_COUNT * _REFRESH_RATE)

/************************************************************************/
/* Local variables                                                      */
/************************************************************************/

/** Luminosity on-going counter, to compare against the led lum */
static uint_fast8_t _lum_count = 0;

/** Scale down from 1024 Hz to 32 for blink control */
static uint_fast8_t _blink_count = 0;

/** Buffer for the DMA transfer */
static volatile uint8_t _spi_dma_tx_buffer[FB_NUMBER_OF_DRIVERS];

/** 
 * Convert a 4 bits light level count into a 5 bits using
 *  a non-linear relashionship x^1.5 * 31 / 15^1.5
 */
static uint_fast8_t _level_looup[] = {
   0, 1, 2, 3, 4, 6, 8, 10, 12, 14, 17, 19, 22, 25, 28, _UPPER_LUM_COUNT-1 };

/************************************************************************/
/* Global variables                                                      */
/************************************************************************/
fb_mem_t fb_current;
fb_mem_t *fb_live;
   
/************************************************************************/
/* Local functions                                                      */
/************************************************************************/	

// Forward declaration
static void _on_timer_tick(void);

/** Prepare the latch pin and start the dma */
static void _initiate_spi_dma_transfer(void)
{
   // Arm the pins latch. It should be low during the data transfer
   ioport_set_pin_level( HC595_LATCH, true );

   // Enable the DMA transfer by enabling the channel
   // The channel turns itself off when all is done
   dma_channel_enable(HC595_DMA_CHANNEL);
}

/**
 * Registered interrupt callback upon dma complete 
 * Latch the data
 */
static void _on_dma_complete(enum dma_channel_status status)
{
   if ( status == DMA_CH_TRANSFER_COMPLETED )
   {
      // Arm the pins latch. It should be low during the data transfer
      ioport_set_pin_level( HC595_LATCH, false );
   }
}

/** Configure the USART as an SPI device */
static void _usart_spi_init(void)
{
   const usart_spi_options_t opt = 
   {
      .baudrate = 1000000,
      .spimode  = 0,
      .data_order = 0
   };
   
   // Configure the usart spi used to talk to the drivers  
   usart_init_spi( &HC595_USART, &opt );
   
   // Turn off the receive (enabled by default)
   usart_rx_disable(&HC595_USART);
}

/** Configure a dedicated timer to manage the frame buffer */
static void _set_timer(void)
{
	// Enable the timer
	tc_enable( &FB_TIMER_TC );

	// Simple timer compare mode required
	tc_set_wgm( &FB_TIMER_TC, TC_WG_NORMAL );

	// The timer service interrupts run at low level to allow
	//  other interrupts to take precedence
   // This interrupt uses up to 50% of the CPU time, so it must be
   //  interruptible
	tc_set_overflow_interrupt_level( &FB_TIMER_TC, TC_INT_LVL_LO );

	// Set the callback
	tc_set_overflow_interrupt_callback( &FB_TIMER_TC, &_on_timer_tick );

	// Set the top to get the correct cycle time around
	tc_write_period( &FB_TIMER_TC, sysclk_get_main_hz() / _CYCLE_RATE );

	// Source is main clock divided by 1
	// This effectively starts the timer ticking
	tc_write_clock_source( &FB_TIMER_TC, TC_CLKSEL_DIV1_gc );
}

/** Configure a DMA channel for copying the fb to the usart-spi */
static void _dma_init(void)
{
   struct dma_channel_config dmach_conf;

   memset(&dmach_conf, 0, sizeof(dmach_conf));
   
   // Enable the DMA controller and associated clocks
   dma_enable();
   
   // 1 byte to copy into the USART Tx buffer at a time
   dma_channel_set_burst_length(&dmach_conf, DMA_CH_BURSTLEN_1BYTE_gc);

   // Send as many bytes as there are drivers
   dma_channel_set_transfer_count(&dmach_conf, FB_NUMBER_OF_DRIVERS);

   // Reload the address once the transaction is over
   dma_channel_set_src_reload_mode(&dmach_conf, DMA_CH_SRCRELOAD_BLOCK_gc);

   // Increment the source address on each new move
   dma_channel_set_src_dir_mode(&dmach_conf, DMA_CH_SRCDIR_INC_gc);

   // Use the spi dma buffer as the source
   dma_channel_set_source_address(&dmach_conf, (uint16_t)(uintptr_t)_spi_dma_tx_buffer);

   // Reload the address once the transaction is over
   dma_channel_set_dest_reload_mode(&dmach_conf, DMA_CH_DESTRELOAD_NONE_gc);

   // Do not change the destination address on each transfer
   dma_channel_set_dest_dir_mode(&dmach_conf, DMA_CH_DESTDIR_FIXED_gc);

   // Transfer into the USART data register
   dma_channel_set_destination_address(&dmach_conf, (uint16_t)(uintptr_t)&(HC595_USART.DATA) );

   // Trigger when the input buffer of the USART is empty
   dma_channel_set_trigger_source(&dmach_conf, DMA_CH_TRIGSRC_USARTC1_DRE_gc);

   // Set a completion callback
   dma_set_callback(HC595_DMA_CHANNEL, &_on_dma_complete);

   // The interrupt is used to latch the data.
   dma_channel_set_interrupt_level(&dmach_conf, DMA_INT_LVL_MED);
   
   // Set the single mode - that is not to depend on a trigger to start, vital!
   dma_channel_set_single_shot(&dmach_conf);

   // Write the config
   dma_channel_write_config(HC595_DMA_CHANNEL, &dmach_conf);
}

/**
 * @details This should be called prior to using any of the framebuffer functions.
 */
void fb_init(void)
{
   // Active all outputs used for driving the hc595
   ioport_set_pin_dir(HC595_TX   , IOPORT_DIR_OUTPUT);
   ioport_set_pin_dir(HC595_CLK  , IOPORT_DIR_OUTPUT);
   ioport_set_pin_dir(HC595_LATCH, IOPORT_DIR_OUTPUT);
   
   // Debug the cycle times
   debug_init(FB);

   // A latch internal zeros
   ioport_set_pin_level(HC595_LATCH, true);

   // Configure the USART transmitter
   _usart_spi_init();

   // Configure the DMA channel to transfer all 6 bytes automatically
   _dma_init();

   // Configure a dedicated timer
   _set_timer();
}

/**
 * Create an offset lookup for the luminosity to spread the current
 * if all LEDs have the same level
 * We use 8 phases
 */
static inline uint8_t _effective_lum_level( const uint8_t lum_count, const uint8_t pos )
{
   return (lum_count + pos) % _UPPER_LUM_COUNT;
}

/**
 * Called to update all 48 possible LEDs at a fast rate.
 * The first cycles are used to control the luminosity/blinking.
 * These are repeated fast to refresh the leds at a flicker free rate.
 * Overall, this function must be very fast.
 * A release build (optimized) can only do 256Hz refresh rate using 50% of the CPU.
 * Therefore, it is important to make it as simple and fast as possible.
 */
static void _on_timer_tick(void)
{
   // Use general purpose registers to speed up processing time
   //  as these require a single cycle for most operations
   register uint_fast8_t driver;
   register uint_fast8_t pos;
   register uint_fast8_t octet;
   uint8_t isDarkMask = 0xff;
   
   fb_led_t led;
   
   // Raise the DEBUG_FB pin to check the execution time
   debug_set(FB);
   
   // Should be turn off altogether?
   if ( measurement_luminosity_is_dark() )
   {
      isDarkMask = 0x00;
   }
   
   // Compute the byte to load for each driver
   for ( driver=0; driver < FB_NUMBER_OF_DRIVERS; ++driver )
   {
      octet = 0;
      
      for ( pos=0; pos < FB_BITS_PER_DRIVER; ++pos )
      {
         // Short hand to the led being addressed
         led = fb_current[(driver<<3) + pos];
         
         // Rotate the single value to prepare for the next bit to come
         octet <<= 1;
         
         // Is this LED on or off - check luminosity
         if
         ( 
            led.status != LED_OFF 
            && 
            _level_looup[led.level] >= _effective_lum_level(_lum_count, pos)
            &&
            ( 
               led.status == LED_ON 
               || 
               (led.status & (_blink_count>>_REFRESH_RATE_POW2) )
            )
         )
         {
            octet |= 1;
         }
      }
      
      // Write to the dma tx buffer
      _spi_dma_tx_buffer[driver] = octet & isDarkMask;
   }

   // Reached the end of the lum cycle?
   if ( ++_lum_count == _UPPER_LUM_COUNT )
   {
      // Apply prescaling to the lum buffer
      ++_blink_count;
      
      _lum_count = 0;
   }

   // Fire the DMA
   _initiate_spi_dma_transfer();
   
   // How long did that take
   debug_clear(FB);
}

/**@}*/
/**@} ---------------------------  End of file  --------------------------- */