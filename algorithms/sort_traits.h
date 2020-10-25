#pragma once
#include <type_traits>
#include <cstdint>



enum class algorithm_tag : uint8_t
{

};



constexpr algorithm_tag operator & (algorithm_tag left, algorithm_tag right)
{
	using T = std::underlying_type_t<algorithm_tag>;
	return (algorithm_tag)((T)left & (T)right);
}

constexpr algorithm_tag operator | (algorithm_tag left, algorithm_tag right)
{
	using T = std::underlying_type_t<algorithm_tag>;
	return (algorithm_tag)((T)left | (T)right);
}

constexpr algorithm_tag operator ^ (algorithm_tag left, algorithm_tag right)
{
	using T = std::underlying_type_t<algorithm_tag>;
	return (algorithm_tag)((T)left ^ (T)right);
}

constexpr algorithm_tag& operator &= (algorithm_tag& left, algorithm_tag right)
{
	using T = std::underlying_type_t<algorithm_tag>;
	(T&)left &= (T)right;
	return left;
}

constexpr algorithm_tag& operator |= (algorithm_tag& left, algorithm_tag right)
{
	using T = std::underlying_type_t<algorithm_tag>;
	(T&)left |= (T)right;
	return left;
}

constexpr algorithm_tag& operator ^= (algorithm_tag& left, algorithm_tag right)
{
	using T = std::underlying_type_t<algorithm_tag>;
	(T&)left ^= (T)right;
	return left;
}