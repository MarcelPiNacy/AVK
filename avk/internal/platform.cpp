#include "platform.h"
#include <Windows.h>

void platform::yield_thread()
{
	(void)SwitchToThread();
}
