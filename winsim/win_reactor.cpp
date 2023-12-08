#include "stdafx.h"
#include <stdint.h>

#include "lib/reactor.h"
#include "lib/alert.h"

// Exit signal for all threads
extern bool AppExitFlag;

extern "C" {

	// Force in the data segmemnt
	volatile reactor_handle_t reactor_notifications = 0;

	/** Current number of handlers */
	static uint8_t _next_handle = 0;

	/** Keep an array of handlers whose position match the bit position of the handle */
	static reactor_handler_t _handlers[REACTOR_MAX_HANDLERS] = { 0 };

   // Timer
   static HANDLE hEvent = NULL;

	/** 
    * Initialize the reactor API
    * Start a thread to simulate the reactor 'main task'
    */
	void reactor_init(void)
	{
      // Create the event to signal processing
      hEvent = CreateEvent(0, FALSE, FALSE, 0);
	}

	void reactor_notify(const reactor_handle_t handle)
	{
		reactor_notifications |= handle;
      alert_and_stop_if(! SetEvent(hEvent));
	}

	/** Add a new reactor process */
	reactor_handle_t reactor_register(const reactor_handler_t handler)
	{
		reactor_handle_t retval;

		_handlers[_next_handle] = handler;
		retval = 1 << _next_handle++;

		return retval;
	}

	/** Process the reactor loop */
	void reactor_run_once(void)
	{
		uint8_t i;
		reactor_handle_t flags;

		// Atomically read and clear the notification flags allowing more
		//  interrupt from setting the flags which will be processed next time round
		while (reactor_notifications)
		{
			flags = reactor_notifications;
			reactor_notifications = 0;

			// Handle the flags
			for (i = 0; i < _next_handle; ++i)
			{
				if (flags & 1)
				{
					// Keep the system alive for as long as the reactor is calling handlers
					// We assume that if no handlers are called, the system is dead.
					_handlers[i]();
				}

				// Move onto next notification
				flags >>= 1;
			}
		};
	}
}

// Task entry to refresh the fb
DWORD WINAPI ReactorTask(LPVOID)
{
   while (!AppExitFlag)
   {
      if (WaitForSingleObject(hEvent, INFINITE) != WAIT_OBJECT_0)
      {
         alert_and_stop();
      }

      reactor_run_once();
   }

   return 0;
}
