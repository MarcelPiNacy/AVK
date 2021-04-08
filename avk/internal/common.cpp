#include "common.h"
#include <cstdio>
#include <cmts.h>

void avk_assertion_handler(const wchar_t* expression)
{
	wprintf_s(L"Expression \"%s\" evaluated to false.", expression);
	if (cmts_is_initialized())
		cmts_terminate(nullptr);
	abort();
}

void avk_assertion_handler(const char* expression)
{
	printf_s("Expression \"%s\" evaluated to false.", expression);
	if (cmts_is_initialized())
		cmts_terminate(nullptr);
	abort();
}
