/*
 * interpolator_fir.c
 *
 *  Created on: Aug 8, 2014
 *      Author: joaorossi
 */

#include "fixed_point.h"

int
interp_init(interp_t *pitrp_interp, q31_t *pq31_coeffs, q31_t *pq31_state, uint32_t u32_L, uint32_t u32_coeffsLen, uint32_t u32_blockLen)
{
	if((pitrp_interp != NULL) && (pq31_coeffs != NULL) && (pq31_state != NULL))
	{
		if((u32_coeffsLen % u32_L) != 0)
		{
			return 1;
		}

		interp_t *pitrp_i = pitrp_interp;
		q31_t *pq31_c = pq31_coeffs;
		q31_t *pq31_s = pq31_state;

		pitrp_i->u32_L = u32_L;
		pitrp_i->u32_phaseLen = u32_coeffsLen/u32_L;
		pitrp_i->u32_blockLen = u32_blockLen;

		pitrp_i->pq31_coeffs = pq31_c;

		memset(pq31_s, 0, (pitrp_i->u32_blockLen + pitrp_i->u32_phaseLen - 1)*sizeof(q31_t));
		pitrp_i->pq31_state = pq31_s;
	}
	else
	{
		return 2;
	}
}

int
interp_flush(interp_t *pitrp_interp)
{
	if(pitrp_interp != NULL)
	{
		interp_t *pitrp_i = pitrp_interp;

		memset(pitrp_i->pq31_state, 0, (pitrp_i->u32_blockLen + pitrp_i->u32_phaseLen - 1)*sizeof(q31_t));

		return 0;
	}
	else
	{
		return 1;
	}
}

int
interp_do(interp_t *pitrp_interp, q31_t *pq31_src, q31_t pq31_dst)
{
	if((pitrp_interp != NULL) && (pq31_src != NULL) && (pq31_dst != NULL))
	{
		interp_t *pitrp_i = pitrp_interp;
		q31_t *pq31_s = pq31_src;
		q31_t *pq31_d = pq31_dst;
		q31_t *pq31_state = pitrp_i->pq31_state;
		q31_t *pq31_coeffs = pitrp_i->pq31_coeffs;
		q63_t q63_acc;
		q31_t q31_x0, q31_c0;
		q31_t *pq31_auxState, *pq31_auxCoeffs;

		uint32_t u32_blockLen = pitrp_i->u32_blockLen;
		uint32_t u32_phaseLen = pitrp_i->u32_phaseLen;

		q31_t pq31_stateCrnt = pitrp_i->pq31_state + ((q31_t) u32_phaseLen - 1);
		uint32_t u32_i, u32_tapCnt;

		while(u32_blockLen > 0)
		{
			*pq31_stateCrnt++ = *pq31_s++;
			u32_i = pitrp_i->u32_L;

			while(u32_i > 0)
			{
				q63_acc = 0;

				pq31_auxState = pq31_state;
				pq31_auxCoeffs = pq31_coeffs + (u32_i - 1);

				u32_tapCnt = u32_phaseLen;

				while(u32_tapCnt > 0)
				{
					q31_c0 = *pq31_auxCoeffs;
					pq31_auxCoeffs += pitrp_i->u32_L;

					q31_x0 = *pq31_auxState++;

					q63_acc += (q63_t) (q31_c0 * q31_x0);

					u32_tapCnt--;
				}

				*pq31_d++ = (q31_t) (q63_acc >> 31);

				u32_i--;
			}

			pq31_state++;

			u32_blockLen--;
		}

		pq31_stateCrnt = pq31_state;

		u32_tapCnt = u32_phaseLen - 1;

		while(u32_tapCnt > 0)
		{
			*pq31_stateCrnt++ = *pq31_state++;

			u32_tapCnt--;
		}

		return 0;
	}
	else
	{
		return 1;
	}
}
