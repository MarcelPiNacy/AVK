#pragma once
#include "common.h"
#include <string_view>

namespace debug
{
#ifdef DEBUG
	void log(const char* message);
	void log(std::string_view message);
#else
	constexpr void log(const char* message) {}
	constexpr void log(std::string_view message) {}
#endif
}