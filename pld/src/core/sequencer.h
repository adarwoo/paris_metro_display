#ifndef sequencer_HAS_ALREADY_BEEN_INCLUDED
#define sequencer_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup core
 * @{
 * @addtogroup sequencer
 * @{
 *****************************************************************************
 * Defines an API for managing the sequencing of the various modes
 *****************************************************************************
 * @file
 * Sequencer API definition
 * @author gax
 */
#ifdef __cplusplus
extern "C" {
#endif

/** Initialise the sequencer. This one will self repeat once after */
void sequencer_start(void);

/** Called to swich modes or sequence type */
void sequencer_switch_short(void);

/** Called to swich modes or sequence type */
void sequencer_switch_long(void);

#ifdef __cplusplus
}
#endif

/**@}*/
/**@}*/
#endif /* ndef sequencer_HAS_ALREADY_BEEN_INCLUDED */