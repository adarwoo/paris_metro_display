#ifndef max1036_h_HAS_ALREADY_BEEN_INCLUDED
#define max1036_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup driver
 * @{
 * @addtogroup max1036
 * @{
 *****************************************************************************
 * Definitions for registers and flags for the
 * - MAX1036
 * - MAX1037
 * - MAX1038
 * - MAX1039 
 * These bits correspond to the configuration and setup bytes organised
 *  respectively as msb and lsb bytes of a 16-bits register.
 * This allow setting the device with one word.
 *****************************************************************************
 * @file
 * Driver for the max1036 declaration
 */
 
 #ifdef __cplusplus
extern "C" {
#endif

/** Initialise the device */
void max1036_init( TWI_t *twi, uint16_t setupAndConfig );

/** Read a single value from the device */
uint8_t max1036_read( TWI_t *twi, bool no_wait);

/** Address is always set and not changeable */
#define MAX1036_I2C_ADDR 0b1100100

/** Register configuration */
typedef enum MAX1036_REGISTER_enum
{
   MAX1036_REGISTER_CONFIG_gc = (0x00<<15), /**< Configuration byte */
   MAX1036_REGISTER_SETUP_gc = (0x01<<7),   /**< Setup byte */
} MAX1036_REGISTER_t;

/** Scan select bits. Two bits select the scanning configuration */
typedef enum MAX1036_SCAN_enum
{
   MAX1036_SCAN_UP0_gc = (0x00<<13),    /**< Scans up from AIN0 to the input selected by CS3–CS0 */
   MAX1036_SCAN_8TIMES_gc = (0x01<<13), /**< Converts the input selected by CS3–CS0 eight times */
   MAX1036_SCAN_UP26_gc = (0x02<<13),   /**< Scans up from AIN2/6 to the input selected by CS1 and CS0 */
   MAX1036_SCAN_SEL_gc = (0x03<<13),    /**< Converts the channel selected by CS3–CS0 */
} MAX1036_SCAN_t;

/** Channel select which analog input channels are to be used for conversion */
typedef enum MAX1036_CHANNEL_enum
{
   MAX1036_CHANNEL_AIN0_gc = (0x00<<9),  /**< Channel AIN0 or AIN0-AIN1 selection */
   MAX1036_CHANNEL_AIN1_gc = (0x01<<9),  /**< Channel AIN1 or AIN1-AIN0 selection */
   MAX1036_CHANNEL_AIN2_gc = (0x02<<9),  /**< Channel AIN2 or AIN2-AIN3 selection */
   MAX1036_CHANNEL_AIN3_gc = (0x03<<9),  /**< Channel AIN3 or AIN3-AIN2 selection */
   MAX1036_CHANNEL_AIN4_gc = (0x04<<9),  /**< Channel AIN4 or AIN4-AIN5 selection */
   MAX1036_CHANNEL_AIN5_gc = (0x05<<9),  /**< Channel AIN5 or AIN5-AIN4 selection */
   MAX1036_CHANNEL_AIN6_gc = (0x06<<9),  /**< Channel AIN6 or AIN6-AIN7 selection */
   MAX1036_CHANNEL_AIN7_gc = (0x07<<9),  /**< Channel AIN7 or AIN7-AIN6 selection */
   MAX1036_CHANNEL_AIN8_gc = (0x08<<9),  /**< Channel AIN8 or AIN8-AIN9 selection */
   MAX1036_CHANNEL_AIN9_gc = (0x09<<9),  /**< Channel AIN9 or AIN9-AIN8 selection */
   MAX1036_CHANNEL_AIN10_gc = (0x0a<<9), /**< Channel AIN10 or AIN10-AIN11 selection */
   MAX1036_CHANNEL_AIN11_gc = (0x0b<<9), /**< Channel AIN11 or AIN11-AIN10 selection */
} MAX1036_CHANNEL_t;

/** Register configuration */
typedef enum MAX1036_MODE_enum
{
   MAX1036_MODE_DIFFERENTIAL_gc = (0x00<<8), /**< Pseudo differential */
   MAX1036_MODE_SINGLE_ENDED_gc = (0x01<<8), /**< Single ended */
   MAX1036_MODE_UNIPOLAR_gc = (0x00<<2),     /**< Unipolar mode */
   MAX1036_MODE_BIPOLAR_gc = (0x01<<2),      /**< Bipolar mode */
} MAX1036_MODE_t;

/** Reference voltage and AIN_/REF state selection */
typedef enum MAX1036_SEL_enum
{                                           /**< Vref   |  AIN_/REF  |Internal Ref state */
   MAX1036_SEL_VDD_AIN_OFF_gc = (0x00<<4),  /**< VDD    |Analog input|Always Off    */
   MAX1036_SEL_EXT_REF_OFF_gc = (0x02<<4),  /**< Ext ref|Ref input   |Always Off    */
   MAX1036_SEL_INT_AIN_AUTO_gc = (0x04<<4), /**< Int ref|Analog input|Auto Shutdown */
   MAX1036_SEL_INT_AIN_ON_gc = (0x05<<4),   /**< Int ref|Analog input|Always On     */
   MAX1036_SEL_INT_REF_ON_gc = (0x06<<4),   /**< Int ref|Ref input   |Always On     */
} MAX1036_SEL_t;

/** Clock selection */
typedef enum MAX1036_CLOCK_enum
{
   MAX1036_CLOCK_INTERNAL_gc = (0x00<<3), /**< Internal clock */
   MAX1036_CLOCK_EXTERNAL_gc = (0x01<<3), /**< Internal clock */
} MAX1036_CLOCK_t;

/** Reset action */
typedef enum MAX1036_RESET_enum
{
   MAX1036_RESET_CONFIG_gc = (0x00<<1), /**< Internal clock */
   MAX1036_RESET_NONE_gc = (0x01<<1),   /**< Internal clock */
} MAX1036_RESET_t;

/** Resets the configuration register to default. Setup register remains */
#define MAX1036_RESET_bm 0x02

#ifdef __cplusplus
}
#endif

/**@}*/
/**@}*/
#endif /* ndef max1036_h_HAS_ALREADY_BEEN_INCLUDED */