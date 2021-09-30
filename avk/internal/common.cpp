#include "common.h"
#include <cstdio>
#include <Comet.hpp>

void avk_assertion_handler(const char* expression)
{
	printf_s("Expression \"%s\" evaluated to false.", expression);
	if (Comet::GetSchedulerState() != Comet::SchedulerState::Uninitialized)
		Comet::Terminate();
	abort();
}

void avk_assertion_handler(const wchar_t* expression)
{
	wprintf_s(L"Expression \"%s\" evaluated to false.", expression);
	if (Comet::GetSchedulerState() != Comet::SchedulerState::Uninitialized)
		Comet::Terminate();
	abort();
}