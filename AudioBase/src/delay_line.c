#include <delay_line.h>

/************************************************
 *					Definitions					*
 ************************************************/

#define MAX_DL_BUFFER_SIZE 256000

/************************************************
 *				Delay Line Structure			*
 ************************************************/

/* Delay line structure */
struct DL {
	/* Delay buffer pointer */
	q31_t    *pq31_buffer;

	/* Read index */
	q31_t    *pq31_read_idx;

	/* Write index */
	q31_t    *pq31_write_idx;

	/* Delay in samples */
	uint32_t u32_delay;

	/* Delay buffer size */
	uint32_t u32_size;
};

/************************************************
 *			Delay Line Implementation			*
 ************************************************/

void *
delay_line_init(uint32_t u32_size)
{
	struct DL *p_dl = calloc(1, sizeof(struct DL));

	if (!p_dl)
		return NULL;

	if ((p_dl->u32_size = u32_size) > MAX_DL_BUFFER_SIZE)
		goto cleanup;

	if (!(p_dl->pq31_buffer = calloc(1, p_dl->u32_size * sizeof(q31_t))))
		goto cleanup;

	p_dl->pq31_read_idx  = p_dl->pq31_buffer;
	p_dl->pq31_write_idx = p_dl->pq31_buffer;

	return p_dl;

cleanup:

	if (p_dl->pq31_buffer)
		free(p_dl->pq31_buffer);

	free(p_dl);

	return NULL;
}

int
delay_line_deinit(void *p_handle)
{
	struct DL *p_dl = p_handle;

	if (!p_dl)
		return -1;

	if (p_dl->pq31_buffer)
		free(p_dl->pq31_buffer);

	free(p_dl);

	return 0;
}

int
delay_line_flush(void *p_handle)
{
	struct DL *p_dl = p_handle;

	if (!p_dl)
		return -1;

	p_dl->pq31_write_idx = p_dl->pq31_buffer;
	p_dl->pq31_read_idx = p_dl->pq31_write_idx - p_dl->u32_delay;

	while (p_dl->pq31_read_idx < p_dl->pq31_buffer)
		p_dl->pq31_read_idx += p_dl->u32_size;

	memset(p_dl->pq31_buffer, 0, p_dl->u32_size * sizeof(q31_t));

	return 0;
}

int
delay_line_set_delay(void *p_handle, uint32_t u32_delay)
{
	struct DL *p_dl = p_handle;

	if (!p_dl)
		return -1;

	if (u32_delay <= p_dl->u32_size)
		p_dl->u32_delay = u32_delay;
	else
		return -1;

	p_dl->pq31_read_idx = p_dl->pq31_write_idx - p_dl->u32_delay;

	while (p_dl->pq31_read_idx < p_dl->pq31_buffer)
		p_dl->pq31_read_idx += p_dl->u32_size;

	return 0;
}

int
delay_line_do(void *p_handle, q31_t *pq31_in, q31_t *pq31_out)
{
	struct DL *p_dl = p_handle;

	if (!p_dl)
		return -1;

	*p_dl->pq31_write_idx++ = *pq31_in;
	*pq31_out = *p_dl->pq31_read_idx++;

	if ((p_dl->pq31_write_idx - p_dl->pq31_buffer) >= p_dl->u32_size)
		p_dl->pq31_write_idx -= p_dl->u32_size;

	if ((p_dl->pq31_read_idx - p_dl->pq31_buffer) >= p_dl->u32_size)
		p_dl->pq31_read_idx -= p_dl->u32_size;

	return 0;
}
