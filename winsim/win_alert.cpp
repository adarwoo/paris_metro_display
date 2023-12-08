#include "stdafx.h"
#include "lib/alert.h"
#include <cstdio>
#include <string>

/** Ready the alert stack */
extern "C" void alert_init(void)
{
   // Nothing to do
}

/** Each application must customize this function */
extern "C" void alert_record(bool do_abort, int line, const char *file)
{
	std::string out("! ");

	out += std::to_string(line) + " " + file + "(" + std::to_string(GetLastError()) + ")";

	// Print to the console for now
	OutputDebugStringA(out.c_str());

	if (do_abort)
	{
		abort();
	}
}
