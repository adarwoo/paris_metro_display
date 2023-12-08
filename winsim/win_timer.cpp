#include "stdafx.h"

#include "lib/timer.h"
#include "lib/alert.h"

// Exit signal for all threads
extern bool AppExitFlag;
static HANDLE hTimer = NULL;
extern "C" void timer_overflow_it();
extern "C" CRITICAL_SECTION TimerCriticalSection;

// Task entry to refresh the fb
DWORD WINAPI TimerTask(LPVOID)
{
   LARGE_INTEGER lElapse;
   lElapse.QuadPart = -1000000LL; // In 100ns interval. 100ms

   hTimer = CreateWaitableTimer(0, false, 0);

   alert_and_stop_if(!hTimer);
   alert_and_stop_if(!SetWaitableTimer(hTimer, &lElapse, 0, 0, 0, FALSE));

   while (!AppExitFlag)
   {
      if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
      {
         alert_and_stop();
      }

      // Rearm
      alert_and_stop_if(!SetWaitableTimer(hTimer, &lElapse, 0, 0, 0, FALSE));

      // Kick the timer as if it was an interrupt
      for (int i = 0; i < 100; ++i)
      {
         timer_overflow_it();
      }
   }

   return 0;
}
