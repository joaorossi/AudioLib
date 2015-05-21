#include <frac_delay_line.h>

/************************************************
 *					Definitions					*
 ************************************************/

#define MAX_DL_BUFFER_SIZE 256000

#define LIN_INTERP(q31_x, q31_y, q31_frc) (MUL__Q31(q31_x, q31_frc) - MUL__Q31(q31_y, q31_frc) + q31_y)

/************************************************
 *		Fractional Delay Line Structure			*
 ************************************************/

/* Fractional delay line structure */
struct FDL {
	/* Fractional delay line buffer pointer */
	q31_t *pq31_buffer;

	/* Delay in samples */
	double f64_delay;

	/* Read position */
	double f64_read_pos;

	/* Write index */
	int32_t i32_write_idx;

	/* Buffer size */
	uint32_t u32_size;
};

/************************************************
 *		Fractional Delay Line Implementation	*
 ************************************************/

void *
frac_delay_line_init(uint32_t u32_size)
{
	struct FDL *p_fdl = calloc(1, sizeof(struct FDL));

	if (!p_fdl)
		return NULL;

	if ((p_fdl->u32_size = u32_size) > MAX_DL_BUFFER_SIZE)
		goto cleanup;

	if (!(p_fdl->pq31_buffer = calloc(1, p_fdl->u32_size * sizeof(q31_t))))
		goto cleanup;

	return p_fdl;

cleanup:

	if (p_fdl->pq31_buffer)
		free(p_fdl->pq31_buffer);

	free(p_fdl);

	return NULL;
}

int
frac_delay_line_deinit(void *p_handle)
{
	struct FDL *p_fdl = p_handle;

	if (!p_fdl)
		return -1;

	if (p_fdl->pq31_buffer)
		free(p_fdl->pq31_buffer);

	free(p_fdl);

	return 0;
}

int
frac_delay_line_flush(void *p_handle)
{
	struct FDL *p_fdl = p_handle;

	if (!p_fdl)
		return -1;

	p_fdl->i32_write_idx = 0;
	p_fdl->f64_read_pos = p_fdl->i32_write_idx - p_fdl->f64_delay;

	while (p_fdl->f64_read_pos < 0.0)
		p_fdl->f64_read_pos += p_fdl->u32_size;

	memset(p_fdl->pq31_buffer, 0, p_fdl->u32_size * sizeof(q31_t));

	return 0;
}

int
frac_delay_line_set_delay(void *p_handle, double f64_delay)
{
	struct FDL *p_fdl = p_handle;

	if (!p_fdl)
		return -1;

	if ((f64_delay <= MAX_DL_BUFFER_SIZE) && (f64_delay >= 0.0))
		p_fdl->f64_delay = f64_delay;
	else
		return -1;

	p_fdl->f64_read_pos = p_fdl->i32_write_idx - p_fdl->f64_delay;

	while (p_fdl->f64_read_pos < 0.0)
		p_fdl->f64_read_pos += p_fdl->u32_size;

	return 0;
}

int
frac_delay_line_do(void *p_handle, q31_t *pq31_in, q31_t *pq31_out)
{
	struct  FDL *p_fdl = p_handle;
	int32_t i32_cur_read_idx, i32_nxt_read_idx;
	double  f64_frac_read_pos;
	q31_t   q31_frac_read_pos, q31_cur_sample, q31_nxt_sample;

	if (!p_fdl || !pq31_in || !pq31_out)
		return -1;

	p_fdl->pq31_buffer[p_fdl->i32_write_idx++] = *pq31_in;

	while (p_fdl->i32_write_idx >= p_fdl->u32_size)
		p_fdl->i32_write_idx -= p_fdl->u32_size;

	i32_cur_read_idx = (int32_t)p_fdl->f64_read_pos;
	i32_nxt_read_idx = i32_cur_read_idx + 1;

	while (i32_nxt_read_idx >= p_fdl->u32_size)
		i32_nxt_read_idx -= p_fdl->u32_size;

	f64_frac_read_pos = p_fdl->f64_read_pos - i32_cur_read_idx;
	q31_frac_read_pos = F64_TO_Q31(f64_frac_read_pos);

	q31_cur_sample = p_fdl->pq31_buffer[i32_cur_read_idx];
	q31_nxt_sample = p_fdl->pq31_buffer[i32_nxt_read_idx];

	*pq31_out = LIN_INTERP(q31_cur_sample, q31_nxt_sample, q31_frac_read_pos);

	p_fdl->f64_read_pos = p_fdl->i32_write_idx - p_fdl->f64_delay;

	while (p_fdl->f64_read_pos < 0.0)
		p_fdl->f64_read_pos += p_fdl->u32_size;

	return 0;
}
