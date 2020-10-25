#pragma once
#include <string>



/// <summary>
/// Halts execution of the sorting algorithm if "condition" turns out to be false.
/// </summary>
/// <param name="condition">The condition to check.</param>
/// <param name="message">An optional error message.</param>
inline void enforce(bool condition, const char* message = nullptr)
{
	if (!condition)
		abort();
}

/// <summary>
/// Halts execution of the sorting algorithm if "condition" turns out to be false.
/// </summary>
/// <param name="condition">The condition to check.</param>
/// <param name="message">An optional error message.</param>
inline void enforce(bool condition, std::string message)
{
	enforce(condition, message.c_str());
}