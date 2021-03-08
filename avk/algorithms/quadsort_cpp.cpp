#include "all.h"
#include <iterator>
#include <cstdint>



namespace quadsort
{
	template <typename I, typename J, typename C>
	struct helper_type
	{
		using traits_type = std::iterator_traits<I>;
		using value_type = typename traits_type::value_type;

		static constexpr void swap_two(I array, C& cmp)
		{
			if (cmp(array, array + 1) > 0)
			{
				value_type swap = array[1];
				array[1] = array[0];
				array[0] = swap;
			}
		}

		static constexpr void swap_three(I array, C& cmp)
		{
			if (cmp(array, array + 1) > 0)
			{
				if (cmp(array, array + 2) <= 0)
				{
					value_type swap = array[0];
					array[0] = array[1];
					array[1] = swap;
				}
				else if (cmp(array + 1, array + 2) > 0)
				{
					value_type swap = array[0];
					array[0] = array[2];
					array[2] = swap;
				}
				else
				{
					value_type swap = array[0];
					array[0] = array[1];
					array[1] = array[2];
					array[2] = swap;
				}
			}
			else if (cmp(array + 1, array + 2) > 0)
			{
				if (cmp(array, array + 2) > 0)
				{
					value_type swap = array[2];
					array[2] = array[1];
					array[1] = array[0];
					array[0] = swap;
				}
				else 
				{
					value_type swap = array[2];
					array[2] = array[1];
					array[1] = swap;
				}
			}
		}

		static constexpr void swap_four(I array, C& cmp)
		{
			if (cmp(array, array + 1) > 0)
			{
				if (cmp(array, array + 2) <= 0)
				{
					value_type swap = array[0];
					array[0] = array[1];
					array[1] = swap;
				}
				else if (cmp(array + 1, array + 2) > 0)
				{
					value_type swap = array[0];
					array[0] = array[2];
					array[2] = swap;
				}
				else
				{
					value_type swap = array[0];
					array[0] = array[1];
					array[1] = array[2];
					array[2] = swap;
				}
			}
			else if (cmp(array + 1, array + 2) > 0)
			{
				if (cmp(array, array + 2) > 0)
				{
					value_type swap = array[2];
					array[2] = array[1];
					array[1] = array[0];
					array[0] = swap;
				}
				else
				{
					value_type swap = array[2];
					array[2] = array[1];
					array[1] = swap;
				}
			}

			if (cmp(array + 1, array + 3) > 0)
			{
				if (cmp(array, array + 3) > 0)
				{
					value_type swap = array[3];
					array[3] = array[2];
					array[2] = array[1];
					array[1] = array[0];
					array[0] = swap;
				}
				else
				{
					value_type swap = array[3];
					array[3] = array[2];
					array[2] = array[1];
					array[1] = swap;
				}
			}
			else if (cmp(array + 2, array + 3) > 0)
			{
				value_type swap = array[3];
				array[3] = array[2];
				array[2] = swap;
			}
		}

		static void tail_insert(I array, value_type& key, uint_fast8_t nmemb, C& cmp)
		{
			I pta = array + nmemb - 1;

			if (cmp(pta, &key) > 0)
			{
				if (nmemb > 4 && cmp(pta - 3, &key) > 0)
				{
					array[nmemb--] = *pta--;
					array[nmemb--] = *pta--;
					array[nmemb--] = *pta--;
					array[nmemb--] = *pta--;
				}

				if (nmemb > 2 && cmp(pta - 2, &key) > 0)
				{
					array[nmemb--] = *pta--;
					array[nmemb--] = *pta--;
				}

				if (nmemb > 1 && cmp(pta - 1, &key) > 0)
				{
					array[nmemb--] = *pta--;
				}

				if (nmemb > 0 && cmp(pta, &key) > 0)
				{
					array[nmemb--] = *pta--;
				}
				array[nmemb] = key;
			}
		}

		static void tail_swap(I array, size_t nmemb, C& cmp)
		{
			value_type pts[8];
			I pta = (I)array;
			uint_fast8_t i, mid, cnt;

			switch (nmemb)
			{
			case 0:
			case 1:
				return;

			case 2:
				swap_two(pta, cmp);
				return;

			case 3:
				swap_three(pta, cmp);
				return;

			case 4:
				swap_four(pta, cmp);
				return;
			}
			swap_four(pta, cmp);

			if (nmemb <= 8)
			{
				for (i = 4; i < nmemb; i++)
				{
					tail_insert(pta, pta[i], i, cmp);
				}
				return;
			}

			pta += 4;

			swap_four(pta, cmp);

			pta += 4;

			switch (nmemb)
			{
			case 9:
				break;

			case 10:
				swap_two(pta, cmp);
				break;

			case 11:
				swap_three(pta, cmp);
				break;

			default:
				swap_four(pta, cmp);

				for (i = 4; i + 8 < nmemb; i++)
				{
					tail_insert(pta, pta[i], i, cmp);
				}
				break;
			}

			pta -= 8;

			if (cmp(&pta[3], &pta[4]) <= 0)
			{
				if (cmp(&pta[7], &pta[8]) <= 0)
				{
					return;
				}
				for (i = 0; i < 8; i++)
				{
					pts[i] = pta[i];
				}
			}
			else if (cmp(&pta[0], &pta[7]) > 0)
			{
				pts[0] = pta[4];
				pts[1] = pta[5];
				pts[2] = pta[6];
				pts[3] = pta[7];

				pts[4] = pta[0];
				pts[5] = pta[1];
				pts[6] = pta[2];
				pts[7] = pta[3];
			}
			else
			{
				cnt = 0;
				i = 0;
				mid = 4;

				pts[cnt++] = cmp(&pta[i], &pta[mid]) > 0 ? pta[mid++] : pta[i++];
				pts[cnt++] = cmp(&pta[i], &pta[mid]) > 0 ? pta[mid++] : pta[i++];
				pts[cnt++] = cmp(&pta[i], &pta[mid]) > 0 ? pta[mid++] : pta[i++];
				pts[cnt++] = cmp(&pta[i], &pta[mid]) > 0 ? pta[mid++] : pta[i++];
				pts[cnt++] = cmp(&pta[i], &pta[mid]) > 0 ? pta[mid++] : pta[i++];

				while (i < 4 && mid < 8)
				{
					if (cmp(&pta[i], &pta[mid]) > 0)
					{
						pts[cnt++] = pta[mid++];
					}
					else
					{
						pts[cnt++] = pta[i++];
					}
				}
				while (i < 4)
				{
					pts[cnt++] = pta[i++];
				}
				while (mid < 8)
				{
					pts[cnt++] = pta[mid++];
				}
			}

			cnt = 0;
			i = 0;
			mid = 8;

			pta[cnt++] = cmp(&pts[i], &pta[mid]) > 0 ? pta[mid++] : pts[i++];

			while (i < 8 && mid < nmemb)
			{
				if (cmp(&pts[i], &pta[mid]) > 0)
				{
					pta[cnt++] = pta[mid++];
				}
				else
				{
					pta[cnt++] = pts[i++];
				}
			}
			while (i < 8)
			{
				pta[cnt++] = pts[i++];
			}
		}

		static size_t quad_swap(I array, size_t nmemb, C& cmp)
		{
			value_type swap[16];
			size_t offset;
			I pta, pts, ptt, pte;
			value_type tmp;

			pta = (I)array;
			pte = pta + nmemb - 4;

			offset = 0;

			while (pta <= pte)
			{
				if (cmp(&pta[0], &pta[1]) > 0)
				{
					if (cmp(&pta[2], &pta[3]) > 0)
					{
						if (cmp(&pta[1], &pta[2]) > 0)
						{
							pts = pta;
							pta += 4;
							goto swapper;
						}
						tmp = pta[2]; pta[2] = pta[3]; pta[3] = tmp;
					}
					tmp = pta[0]; pta[0] = pta[1]; pta[1] = tmp;
				}
				else if (cmp(&pta[2], &pta[3]) > 0)
				{
					tmp = pta[2]; pta[2] = pta[3]; pta[3] = tmp;
				}

				if (cmp(&pta[1], &pta[2]) > 0)
				{
					tmp = pta[1];

					if (cmp(&pta[0], &pta[2]) <= 0)
					{
						if (cmp(&pta[1], &pta[3]) <= 0)
						{
							pta[1] = pta[2]; pta[2] = tmp;
						}
						else
						{
							pta[1] = pta[2]; pta[2] = pta[3]; pta[3] = tmp;
						}
					}
					else if (cmp(&pta[0], &pta[3]) > 0)
					{
						pta[1] = pta[3]; pta[3] = tmp; tmp = pta[0]; pta[0] = pta[2]; pta[2] = tmp;
					}
					else if (cmp(&pta[1], &pta[3]) <= 0)
					{
						pta[1] = pta[0]; pta[0] = pta[2]; pta[2] = tmp;
					}
					else
					{
						pta[1] = pta[0]; pta[0] = pta[2]; pta[2] = pta[3]; pta[3] = tmp;
					}
				}
				pta += 4;

				continue;

			swapper:

				if (pta <= pte)
				{
					if (cmp(&pta[0], &pta[1]) > 0)
					{
						if (cmp(&pta[2], &pta[3]) > 0)
						{
							if (cmp(&pta[1], &pta[2]) > 0)
							{
								if (cmp(&pta[-1], &pta[0]) > 0)
								{
									pta += 4;

									goto swapper;
								}
							}
							tmp = pta[2]; pta[2] = pta[3]; pta[3] = tmp;
						}
						tmp = pta[0]; pta[0] = pta[1]; pta[1] = tmp;
					}
					else if (cmp(&pta[2], &pta[3]) > 0)
					{
						tmp = pta[2]; pta[2] = pta[3]; pta[3] = tmp;
					}

					if (cmp(&pta[1], &pta[2]) > 0)
					{
						if (cmp(&pta[0], &pta[2]) <= 0)
						{
							if (cmp(&pta[1], &pta[3]) <= 0)
							{
								tmp = pta[1]; pta[1] = pta[2]; pta[2] = tmp;
							}
							else
							{
								tmp = pta[1]; pta[1] = pta[2]; pta[2] = pta[3]; pta[3] = tmp;
							}
						}
						else if (cmp(&pta[0], &pta[3]) > 0)
						{
							tmp = pta[0]; pta[0] = pta[2]; pta[2] = tmp;
							tmp = pta[1]; pta[1] = pta[3]; pta[3] = tmp;

						}
						else if (cmp(&pta[1], &pta[3]) <= 0)
						{
							tmp = pta[0]; pta[0] = pta[2]; pta[2] = pta[1]; pta[1] = tmp;
						}
						else
						{
							tmp = pta[0]; pta[0] = pta[2]; pta[2] = pta[3]; pta[3] = pta[1]; pta[1] = tmp;
						}
					}

					if (pts == array)
					{
						offset = (pta - pts) / 16 * 16;
					}
					ptt = pta - 1;
					pta += 4;
				}
				else
				{
					if (pts == array)
					{
						switch (nmemb % 4)
						{
						case 3:
							if (cmp(&pta[2], &pta[3]) <= 0)
							{
								offset = (pta - pts) / 16 * 16;
								break;
							}
						case 2:
							if (cmp(&pta[1], &pta[2]) <= 0)
							{
								offset = (pta - pts) / 16 * 16;
								break;
							}
						case 1:
							if (cmp(&pta[0], &pta[1]) <= 0)
							{
								offset = (pta - pts) / 16 * 16;
								break;
							}
						case 0:
							goto swapped;
						}
					}
					ptt = pta - 1;
				}

				while (pts < ptt)
				{
					tmp = *pts;
					*pts++ = *ptt;
					*ptt-- = tmp;
				}
				continue;

			swapped:

				ptt = pts + nmemb - 1;

				while (pts < ptt)
				{
					tmp = *pts;
					*pts++ = *ptt;
					*ptt-- = tmp;
				}
				return 1;
			}

			tail_swap(pta, nmemb % 4, cmp);

			// block 4 quadmerge

			pta = pte = (I)array + offset;

			for (size_t i = (nmemb - offset) / 16; i > 0; --i, pte += 16)
			{
				if (cmp(&pta[3], &pta[4]) <= 0)
				{
					if (cmp(&pta[11], &pta[12]) <= 0)
					{
						if (cmp(&pta[7], &pta[8]) <= 0)
						{
							pta += 16;
							continue;
						}
						pts = swap;

						*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
						*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
						*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
						*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

						goto step3;
					}
					pts = swap;

					*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
					*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

					goto step2;
				}

				// step1:

				pts = swap;

				if (cmp(&pta[3], &pta[7]) <= 0)
				{
					ptt = pta + 4;

					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

					while (pta < pte + 4)
					{
						*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					}
					while (ptt < pte + 8)
					{
						*pts++ = *ptt++;
					}
					pta = ptt;
				}
				else if (cmp(&pta[0], &pta[7]) > 0)
				{
					ptt = pta + 4;
					*pts++ = *ptt++; *pts++ = *ptt++; *pts++ = *ptt++; *pts++ = *ptt++;
					*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
					pta = ptt;
				}
				else
				{
					ptt = pta + 4;

					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

					while (ptt < pte + 8)
					{
						*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					}

					while (pta < pte + 4)
					{
						*pts++ = *pta++;
					}

					pta = ptt;
				}

				if (cmp(&pta[3], &pta[4]) <= 0)
				{
					ptt = pta;
					pts = swap;
					pta = pte;

					if (cmp(&pts[7], &ptt[0]) <= 0)
					{
						*pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++;
						*pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++;

						pta += 8;
					}
					else if (cmp(&pts[7], &ptt[7]) <= 0)
					{
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

						while (pts < swap + 8)
						{
							*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						}

						pta = pte + 16;
					}
					else if (cmp(&pts[0], &ptt[7]) > 0)
					{
						*pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++;
						*pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++;
					}
					else
					{
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

						while (ptt < pte + 16)
						{
							*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
						}

						while (pts < swap + 8)
						{
							*pta++ = *pts++;
						}
					}
					continue;
				}

			step2:

				if (cmp(&pta[3], &pta[7]) <= 0)
				{
					ptt = pta + 4;

					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

					while (pta < pte + 12)
					{
						*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					}
					while (pts < swap + 16)
					{
						*pts++ = *ptt++;
					}
				}
				else if (cmp(&pta[0], &pta[7]) > 0)
				{
					ptt = pta + 4;

					*pts++ = *ptt++; *pts++ = *ptt++; *pts++ = *ptt++; *pts++ = *ptt++;
					*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
				}
				else
				{
					ptt = pta + 4;

					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

					while (ptt < pte + 16)
					{
						*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
					}
					while (pts < swap + 16)
					{
						*pts++ = *pta++;
					}
				}

			step3:

				pta = pte;
				pts = swap;

				if (cmp(&pts[7], &pts[15]) <= 0)
				{
					ptt = pts + 8;

					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

					//			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

					while (pts < swap + 8)
					{
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					}
					while (ptt < swap + 16)
					{
						*pta++ = *ptt++;
					}
				}
				else if (cmp(&pts[0], &pts[15]) > 0)
				{
					ptt = pts + 8;

					*pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++;
					*pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++; *pta++ = *ptt++;

					*pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++;
					*pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++;
				}
				else
				{
					ptt = pts + 8;

					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

					*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

					while (ptt < swap + 16)
					{
						*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
					}
					while (pts < swap + 8)
					{
						*pta++ = *pts++;
					}
				}
			}

			if (nmemb % 16 > 4)
			{
				tail_merge(pta, swap, nmemb % 16, 4, cmp);
			}
			return 0;
		}

		static void quad_merge(I array, J swap, size_t nmemb, size_t block, C& cmp)
		{
			size_t offset;
			I pta, pts, c, c_max, d, d_max;

			while (block * 8 < nmemb)
			{
				offset = 0;

				while (offset + block * 4 <= nmemb)
				{
					pta = (I)array + offset;

					c_max = pta + block;

					if (cmp(c_max - 1, c_max) <= 0)
					{
						c_max = pta + block * 3;

						if (cmp(c_max - 1, c_max) <= 0)
						{
							c_max = pta + block * 2;

							if (cmp(c_max - 1, c_max) <= 0)
							{
								offset += block * 4;
								continue;
							}
							pts = swap;

							c = pta;

							while (c < c_max - 8)
							{
								*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
								*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
							}
							while (c < c_max)
							{
								*pts++ = *c++;
							}
							c = c_max;
							c_max = c + block * 2;

							while (c < c_max - 8)
							{
								*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
								*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
							}
							while (c < c_max)
							{
								*pts++ = *c++;
							}
							goto step3;
						}
						pts = swap;

						c = pta;
						c_max = pta + block * 2;

						while (c < c_max)
						{
							*pts++ = *c++;
						}
						goto step2;
					}

					// step1:

					pts = swap;

					c = pta;

					d = c_max;
					d_max = d + block;

					if (cmp(c_max - 1, d_max - 1) <= 0)
					{
						while (c < c_max)
						{
							while (cmp(c, d) > 0)
							{
								*pts++ = *d++;
							}
							*pts++ = *c++;
						}
						while (d < d_max)
						{
							*pts++ = *d++;
						}
					}
					else
					{
						while (d < d_max)
						{
							while (cmp(c, d) <= 0)
							{
								*pts++ = *c++;
							}
							*pts++ = *d++;
						}
						while (c < c_max)
						{
							*pts++ = *c++;
						}
					}

				step2:

					c = pta + block * 2;

					c_max = c + block;
					d = c_max;
					d_max = d + block;

					if (cmp(c_max - 1, d_max - 1) <= 0)
					{
						while (c < c_max)
						{
							while (cmp(c, d) > 0)
							{
								*pts++ = *d++;
							}
							*pts++ = *c++;
						}
						while (d < d_max)
						{
							*pts++ = *d++;
						}
					}
					else
					{
						while (d < d_max)
						{
							while (cmp(c, d) <= 0)
							{
								*pts++ = *c++;
							}
							*pts++ = *d++;
						}

						while (c < c_max)
						{
							*pts++ = *c++;
						}
					}

				step3:

					pts = swap;

					c = pts;

					c_max = c + block * 2;

					d = c_max;
					d_max = d + block * 2;

					if (cmp(c_max - 1, d_max - 1) <= 0)
					{
						while (c < c_max)
						{
							while (cmp(c, d) > 0)
							{
								*pta++ = *d++;
							}
							*pta++ = *c++;
						}
						while (d < d_max)
						{
							*pta++ = *d++;
						}
					}
					else
					{
						while (d < d_max)
						{
							while (cmp(d, c) > 0)
							{
								*pta++ = *c++;
							}
							*pta++ = *d++;
						}
						while (c < c_max)
						{
							*pta++ = *c++;
						}
					}
					offset += block * 4;
				}
				tail_merge((I)array + offset, swap, nmemb - offset, block, cmp);

				block *= 4;
			}
			tail_merge(array, swap, nmemb, block, cmp);
		}

		static void tail_merge(I array, J swap, size_t nmemb, size_t block, C& cmp)
		{
			unsigned int offset;
			I pta, pts, c, c_max, d, d_max, e;

			pts = swap;

			while (block < nmemb)
			{
				for (offset = 0; offset + block < nmemb; offset += block * 2)
				{
					pta = (I)array + offset;

					c_max = pta + block;

					if (cmp(c_max - 1, c_max) <= 0)
					{
						continue;
					}

					if (offset + block * 2 <= nmemb)
					{
						d_max = pta + block * 2;
					}
					else
					{
						d_max = (I)array + nmemb;
					}

					if (offset + block * 2 <= nmemb)
					{
						c_max = pts + block;
					}
					else
					{
						c_max = pts + nmemb - (offset + block);
					}

					d = d_max - 1;
					e = pta + block - 1;

					while (cmp(e, d) <= 0)
					{
						d_max--;
						d--;
						c_max--;
					}

					c = c_max - 1;

					while (c >= pts + 8)
					{
						*c-- = *d--; *c-- = *d--; *c-- = *d--; *c-- = *d--; *c-- = *d--; *c-- = *d--; *c-- = *d--; *c-- = *d--;
					}

					while (c >= pts)
					{
						*c-- = *d--;
					}

					c = c_max - 1;
					d = pta + block - 1;
					e = d_max - 1;

					if (cmp(pta, pta + block) <= 0)
					{
						*e-- = *d--;

						while (c >= pts)
						{
							while (cmp(d, c) > 0)
							{
								*e-- = *d--;
							}
							*e-- = *c--;
						}
					}
					else
					{
						*e-- = *d--;

						while (d >= pta)
						{
							while (cmp(d, c) <= 0)
							{
								*e-- = *c--;
							}
							*e-- = *d--;
						}
						while (c >= pts)
						{
							*e-- = *c--;
						}
					}
				}
				block *= 2;
			}
		}

		static constexpr void sort(I begin, I end, C&& cmp)
		{
			const auto nmemb = ::std::distance(begin, end);
			if (nmemb < 16)
			{
				tail_swap(begin, (unsigned char)nmemb, cmp);
			}
			else if (nmemb < 128)
			{
				if (quad_swap(begin, nmemb, cmp) == 0)
				{
					value_type swap[64];
					tail_merge(begin, swap, nmemb, 16, cmp);
				}
			}
			else
			{
				if (quad_swap(begin, nmemb, cmp) == 0)
				{
					value_type* buffer = new value_type[nmemb / 2];
					assert(buffer != nullptr);
					quad_merge(begin, buffer, nmemb, 16, cmp);
					delete[] buffer;
				}
			}
		}
	};

	template <typename RandomAccessIterator, typename Compare>
	void sort(RandomAccessIterator begin, RandomAccessIterator end, Compare&& compare)
	{
		return helper_type<RandomAccessIterator, RandomAccessIterator, Compare>::sort(begin, end, std::forward<Compare>(compare));
	}
}


void quadsort_cpp(main_array array)
{
	return quadsort::sort(array.begin(), array.end(), [](item* left, item* right)
	{
		return compare(*left, *right);
	});
}