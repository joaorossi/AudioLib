#include <wave_table_osc.h>
#include <math.h>

/************************************************
 *					Definitions					*
 ************************************************/

#define MAX_WAVE_TABLE_SIZE 65536

enum {
	SIN = 0,
	TRI,
	SAW,
	SQR
};

/************************************************
 *				Delay Line Structure			*
 ************************************************/

struct WTO {
	/* Table */
	q31_t    *pq31_table;

	/* Increment, position and offset parameters */
	double   f64_incr, f64_pos, f64_offset;

	/* Table type */
	uint32_t u32_type;

	/* Table size */
	uint32_t u32_size;
};

/************************************************
 *			Delay Line Implementation			*
 ************************************************/

void *
wave_table_osc_init(uint32_t u32_size)
{
	struct WTO *p_wto = calloc(1, sizeof(struct WTO));

	if (!p_wto)
		return NULL;

	if ((p_wto->u32_size = u32_size) > MAX_WAVE_TABLE_SIZE)
		goto cleanup;

	if (!(p_wto->pq31_table = calloc(1, p_wto->u32_size * sizeof(q31_t))))
		goto cleanup;

	return p_wto;

cleanup:

	if (p_wto->pq31_table)
		free(p_wto->pq31_table);

	free(p_wto);

	return NULL;
}

int
wave_table_osc_deinit(void *p_handle)
{
	struct WTO *p_wto = p_handle;

	if (!p_wto)
		return -1;

	if (!p_wto->pq31_table)
		return -1;

	free(p_wto->pq31_table);
	free(p_wto);

	return 0;
}

int
wave_table_osc_flush(void *p_handle)
{
	struct WTO *p_wto = p_handle;

	if (!p_wto)
		return -1;

	p_wto->f64_pos = p_wto->f64_offset;

	return 0;
}

int
wave_table_osc_set_type(void *p_handle, uint32_t u32_type)
{
	struct WTO *p_wto = p_handle;

	uint32_t u32_i;
	double   f64_aux;

	if (!p_wto)
		return -1;

	if (!p_wto->pq31_table)
		return -1;

	if (u32_type > SQR)
		return -1;

	p_wto->u32_type = u32_type;

	switch (p_wto->u32_type) {
		case SIN:
			for (u32_i = 0; u32_i < p_wto->u32_size; u32_i++) {
				f64_aux = sin(2.0 * M_PI * (((double)u32_i)/((double)p_wto->u32_size)));
				p_wto->pq31_table[u32_i] = F64_TO_Q31(f64_aux);
			}
			break;

		case TRI:
			for (u32_i = 0; u32_i < p_wto->u32_size; u32_i++) {
				if (u32_i < (0.25 * p_wto->u32_size))
					f64_aux = (4.0 * (double)u32_i)/((double)p_wto->u32_size);
				else if ((u32_i >= (0.25 * ((double)p_wto->u32_size))) && (u32_i < (0.75 * ((double)p_wto->u32_size))))
					f64_aux = (-4.0 * (double)u32_i)/((double)p_wto->u32_size) + 2.0;
				else if (u32_i >= ((0.75 * ((double)p_wto->u32_size)) && (u32_i < p_wto->u32_size)))
					f64_aux = (4.0 * (double)u32_i)/((double)p_wto->u32_size) - 4.0;
				else
					return -1; /* should never get here */

				p_wto->pq31_table[u32_i] = F64_TO_Q31(f64_aux);
			}
			break;

		case SAW:
			for (u32_i = 0; u32_i < p_wto->u32_size; u32_i++) {
				f64_aux = (2.0 * (double)u32_i)/((double)p_wto->u32_size) - 1.0;
				p_wto->pq31_table[u32_i] = F64_TO_Q31(f64_aux);
			}
			break;

		case SQR:
			for (u32_i = 0; u32_i < p_wto->u32_size; u32_i++) {
				if (u32_i < (0.5 * p_wto->u32_size))
					p_wto->pq31_table[u32_i] = 0x7fffffff;
				else if ((u32_i >= (0.5 * p_wto->u32_size)) && (u32_i < p_wto->u32_size))
					p_wto->pq31_table[u32_i] = 0x80000000;
				else
					return -1; /* should never get here */
			}
			break;

		default:
			return -1;
	}

	return 0;
}

int
wave_table_osc_set_incr(void *p_handle, double f64_incr)
{
	struct WTO *p_wto = p_handle;

	if (!p_wto)
		return -1;

	if ((f64_incr < 0.0) || (f64_incr >= p_wto->u32_size))
		return -1;

	p_wto->f64_incr = f64_incr;

	return 0;
}

int
wave_table_osc_set_offset(void *p_handle, double f64_offset)
{
	struct WTO *p_wto = p_handle;

	if (!p_wto)
		return -1;

	if ((f64_offset >= p_wto->u32_size) || (f64_offset < 0.0))
		return -1;

	p_wto->f64_pos -= p_wto->f64_offset;
	p_wto->f64_offset = f64_offset;
	p_wto->f64_pos += p_wto->f64_offset;

	if (p_wto->f64_pos < 0.0)
		p_wto->f64_pos += p_wto->u32_size - 1;

	if (p_wto->f64_pos >= (p_wto->u32_size - 1))
		p_wto->f64_pos -= p_wto->u32_size - 1;

	return 0;
}

int
wave_table_osc_do(void *p_handle, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct WTO *p_wto = p_handle;

	q31_t   q31_cur, q31_nxt, q31_frc_pos;
	int32_t i32_int_pos;
	double  f64_frc_pos;

	if (!p_wto || !pq31_dst)
		return -1;

	if (!p_wto->pq31_table)
		return -1;

	do {
		i32_int_pos = (int32_t)p_wto->f64_pos;
		f64_frc_pos = p_wto->f64_pos - i32_int_pos;
		q31_frc_pos = F64_TO_Q31(f64_frc_pos);

		q31_cur = p_wto->pq31_table[i32_int_pos];

		if (i32_int_pos >= (p_wto->u32_size - 1))
			q31_nxt = p_wto->pq31_table[0];
		else
			q31_nxt = p_wto->pq31_table[i32_int_pos + 1];

		q31_nxt -= q31_cur;
		q31_nxt = MUL__Q31(q31_nxt, q31_frc_pos);

		*pq31_dst++ = q31_cur + q31_nxt;

		if ((p_wto->f64_pos += p_wto->f64_incr) >= p_wto->u32_size)
			p_wto->f64_pos -= p_wto->u32_size;
	} while (--u32_buffer_size);

	return 0;
}
