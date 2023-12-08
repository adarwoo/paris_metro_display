#include "stdafx.h"
#include "driver/fb.h"
#include "winsim.h"
#include "lib/alert.h"

/** Upper limit for the luminosity duty cycle */
#define _REFRESH_RATE 32
#define _UPPER_LUM_COUNT 32

/************************************************************************/
/* Local variables                                                      */
/************************************************************************/

/** Scale down from 1024 Hz to 32 for blink control */
static uint_fast8_t _blink_count = 0;

/** Buffer for the DMA transfer */
int FbLeds[48];

/** State of all the leds in the system */
extern "C"
{
	fb_mem_t fb_current;

	/** Current working copy of the buffer. This is to prevent glitches. */
	fb_mem_t *fb_live;

	/** Initialise the framebuffer API */
	void fb_init(void)
	{
		memset((void *)FbLeds, 0, sizeof(FbLeds));
	}
}

static uint8_t cycle_counter = 0;
static HANDLE hTimer = NULL;

extern bool AppExitFlag;

// Task entry to refresh the fb
DWORD WINAPI FbTask(LPVOID lpParam)
{
	auto pldSim = reinterpret_cast<PldSimulator *>(lpParam);
	LARGE_INTEGER lElapse;
	lElapse.QuadPart = -312500LL; // In 100ns interval. 1/32s

	hTimer = CreateWaitableTimer(0, false, 0);

	alert_and_stop_if(!hTimer);
	alert_and_stop_if(!SetWaitableTimer(hTimer, &lElapse, 0, 0, 0, FALSE));
	
	while ( !AppExitFlag)
	{
		if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
		{
			alert_and_stop();
		}

		// Rearm
		alert_and_stop_if(!SetWaitableTimer(hTimer, &lElapse, 0, 0, 0, FALSE));

		for (register uint_fast8_t i = 0; i < 48; ++i)
		{
			// Short hand to the led being addressed
			register fb_led_t led = fb_current[i];

			// Is this LED on or off - check luminosity
			if (led.status == LED_ON || (led.status & (_blink_count>>1)))
			{
				FbLeds[i] = led.level;
			}
			else
			{
				FbLeds[i] = 0;
			}
		}

		// Apply prescaling to the lum buffer
		_blink_count = ++_blink_count % 32;

		// Refresh the display
		pldSim->DrawLeds();
	}

	return 0;
}
