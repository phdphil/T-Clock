#include "clock.h"

int LoadClockAPI(const char* dll_path, TClockAPI* api){
	typedef int (*SetupClockAPI_t)(int version, TClockAPI* api);
	SetupClockAPI_t SetupClockAPI;
	HMODULE dll=LoadLibrary(dll_path); // leak / auto-free on process exit
	SetupClockAPI=(SetupClockAPI_t)GetProcAddress(dll,"SetupClockAPI");
	if(!SetupClockAPI)
		return 1;
	return SetupClockAPI(CLOCK_API, api);
}
