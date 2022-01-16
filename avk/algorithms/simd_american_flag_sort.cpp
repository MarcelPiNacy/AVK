#include "all.h"

#include <iterator>
#ifdef _MSVC_LANG
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <immintrin.h>
#else
#error ""
#endif



template <typename I, typename F>
struct SIMDRadixSorter
{
#ifdef _MSVC_LANG
	using XMM = __m128i;
#elif defined(__GNUC__) || defined(__clang__)
	using XMM = __m128i;
#endif

	// Returns whether all keys have the same radix.
	static bool Count(I begin, I end, F& extract, uint32_t digit_index, uint32_t* out)
	{
		const uint32_t byte_index = digit_index >> 2;
		const uint32_t shift = (digit_index & 3) << 1;

		// Count keys, 4 at a time:
		while (std::distance(begin, end) >= 4)
		{
			++out[(extract(*begin++, byte_index) >> shift) & 3];
			++out[(extract(*begin++, byte_index) >> shift) & 3];
			++out[(extract(*begin++, byte_index) >> shift) & 3];
			++out[(extract(*begin++, byte_index) >> shift) & 3];
		}
		
		// Count remaining:
		for (; begin != end; ++begin)
			++out[(extract(*begin++, byte_index) >> shift) & 3];

		return __popcnt(15U & ~_mm_movemask_epi8(_mm_cmpeq_epi32(_mm_load_si128((const XMM*)out), _mm_setzero_si128()))) == 1;
	}

	static void PrefixSum(const uint32_t* counts, uint32_t* out)
	{
		XMM vec = _mm_load_si128((const XMM*)counts);
		vec = _mm_add_epi32(vec, _mm_slli_si128(vec, 4));
		vec = _mm_add_epi32(vec, _mm_slli_si128(vec, 8));
		vec = _mm_slli_si128(vec, 4);
		_mm_store_si128((XMM*)counts, vec);
	}

	static void UnShuffle(
		I begin, I end, 
		const uint32_t* offsets_source,
		F& extract, uint32_t digit_index)
	{
		const uint32_t byte_index = digit_index >> 2;
		const uint32_t shift = (digit_index & 3) << 1;

		I offsets[4] = {};
		I cursors[4] = {};

		for (uint8_t i = 0; i != 4; ++i)
			offsets[i] = cursors[i] = begin + offsets_source[i];

		for (uint8_t i = 0; i != 3; )
		{
			if (begin >= offsets[i + 1])
			{
				++i;
				continue;
			}
			uint8_t radix = (extract(*begin++, byte_index) >> shift) & 3;
			if (radix == i)
			{
				++begin;
				continue;
			}
			std::iter_swap(begin, cursors[radix]);
			++cursors[radix];
		}
	}

	static void Core(I begin, I end, F& extract, uint32_t digit_index)
	{
		alignas (16) uint32_t counts[4] = {};
		alignas (16) uint32_t offsets[4] = {};
		
		if (Count(begin, end, extract, digit_index, counts))
		{
			PrefixSum(counts, offsets);
			UnShuffle(begin, end, offsets, extract, digit_index);
		}

		if (digit_index == 0)
			return;
		--digit_index;

		const I lhs[4] =
		{
			begin + offsets[0],
			begin + offsets[1],
			begin + offsets[2],
			begin + offsets[3],
		};

		const I rhs[4] =
		{
			lhs[0] + counts[0],
			lhs[1] + counts[1],
			lhs[2] + counts[2],
			lhs[3] + counts[3],
		};

		for (uint8_t i = 0; i != 4; ++i)
			Core(lhs[0], rhs[0], extract, digit_index);
	}

	static void Sort(I begin, I end, F&& extract, uint32_t max_digits)
	{
		Core(begin, end, extract, max_digits - 1);
	}
};



void simd_american_flag_sort(main_array array)
{
	SIMDRadixSorter<item*, decltype(extract_byte)*> sorter = {};
	sorter.Sort(array.begin(), array.end(), extract_byte, 16);
}