#pragma once
#include <new>



namespace platform
{
	constexpr auto cache_line_size = std::hardware_constructive_interference_size;
	constexpr auto page_size = 4096; //TODO: compute dynamically?
}