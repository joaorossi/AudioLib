/*
 * fixed_point.c
 *
 *  Created on: Jul 22, 2014
 *      Author: joaorossi
 */

#include <fixedpoint.h>

int
convert_f32_to_q31(f32_t *pf32_src, q31_t *pq31_dst, uint32_t u32_size)
{
	if((pf32_src != NULL) && (pq31_dst != NULL)) {
		f32_t *pf32_s = pf32_src;
		q31_t *pq31_d = pq31_dst;

		while(u32_size > 0) {
			*pq31_d++ = lrintf((*pf32_s++)*F32_Q31_CONV);
			u32_size--;
		}

		return 0;
	} else {
		return 1;
	}
}


int
convert_f64_to_q31(f64_t *pf64_src, q31_t *pq31_dst, uint32_t u32_size)
{
	if((pf64_src != NULL) && (pq31_dst != NULL)) {
		f64_t *pf64_s = pf64_src;
		q31_t *pq31_d = pq31_dst;

		while(u32_size > 0) {
			*pq31_d++ = lrint((*pf64_s++)*F64_Q31_CONV);
			u32_size--;
		}

		return 0;
	} else {
		return 1;
	}
}


int
convert_q15_to_q31(q15_t *pq15_src, q31_t *pq31_dst, uint32_t u32_size)
{
	if((pq15_src != NULL) && (pq31_dst != NULL)) {
		q15_t *pq15_s = pq15_src;
		q31_t *pq31_d = pq31_dst;

		while(u32_size) {
			*pq31_d++ = ((q31_t)*pq15_s++) << 16;
			u32_size--;
		}

		return 0;
	} else {
		return 1;
	}
}


int
convert_q31_to_q15(q31_t *pq31_src, q15_t *pq15_dst, uint32_t u32_size)
{
	if((pq31_src != NULL) && (pq15_dst != NULL)) {
		q31_t *pq31_s = pq31_src;
		q15_t *pq15_d = pq15_dst;

	while(u32_size) {
		*pq15_d++ = (q15_t)((*pq31_s++) >> 16);
		u32_size--;
	}

	return 0;
	} else {
		return 1;
	}
}
