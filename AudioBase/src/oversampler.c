#include <oversampler.h>

/************************************************
 *					Definitions					*
 ************************************************/

/************************************************
 *			Oversampler Structure				*
 ************************************************/

/* Convolution ring buffer structure */
struct CRB {

	/* Buffer length */
	uint32_t	u32_buff_len;

	/* R/W indexes */
	int32_t		i32_rd_idx;
	int32_t		i32_wr_idx;

	/* Buffer */
	q31_t		*pq31_ring_buffer;

};

/* Oversampler structure */
struct OVRSMP {

	/* Original sample rate */
	f64_t		f64_orig_sample_rate;

	/* Oversampled rate */
	f64_t		f64_ovrsmp_rate;

	/* Resample factor - pwr of 2 always!! */
	uint32_t	u32_ovrsmp_factor;

	/* Original sample rate buffer len */
	uint32_t	u32_orig_buffer_len;

	/* Impulse response */
	uint32_t	u32_ir_len;
	q31_t		*pq31_ir;

	/* Polyphase filterbank */
	uint32_t	u32_phase_ir_len;
	q31_t		**pq31_filterbank;

	/* Convolution ring buffers */
	struct CRB	**pcrb_fir_hist;

	/* Oversampled buffer */
	uint32_t	u32_ovrsmp_buff_len;
	q31_t		*pq31_interp_buffer;
	q31_t		*pq31_decimt_buffer;

};

/************************************************
 *			Static helper functions				*
 ************************************************/

static int
_crb_pop_smp(struct CRB *pcrb_buff, q31_t *pq31_out)
{
	if (!pcrb_buff || !pq31_out)
		return -1;

	*pq31_out = pcrb_buff->pq31_ring_buffer[pcrb_buff->i32_rd_idx];

	if (--(pcrb_buff->i32_rd_idx) < 0)
		pcrb_buff->i32_rd_idx += pcrb_buff->u32_buff_len;

	return 0;
}

static int
_crb_push_smp(struct CRB *pcrb_buff, q31_t *pq31_in)
{
	if (!pcrb_buff || !pq31_in)
		return -1;

	if (--(pcrb_buff->i32_wr_idx) < 0)
		pcrb_buff->i32_wr_idx += pcrb_buff->u32_buff_len;
	pcrb_buff->i32_rd_idx = pcrb_buff->i32_wr_idx;

	pcrb_buff->pq31_ring_buffer[pcrb_buff->i32_wr_idx] = *pq31_in;

	return 0;
}

static int

_convolute(struct CRB *pcrb_fir_hist, q31_t *pq31_phase_ir, q31_t *pq31_x, q31_t *pq31_y)
{
	uint32_t u32_i;

	q31_t pq31_hist[1];

	q63_t q63_acc = 0;

	if (!pcrb_fir_hist || !pq31_phase_ir || !pq31_x || !pq31_y)
		return -1;

	if (_crb_push_smp(pcrb_fir_hist, pq31_x))
		return -1;

	for (u32_i = 0; u32_i < pcrb_fir_hist->u32_buff_len; u32_i++) {
		if (_crb_pop_smp(pcrb_fir_hist, pq31_hist))
			return -1;

		MAC__Q63_Q31_Q31(q63_acc, *pq31_hist, pq31_phase_ir[u32_i]);
	}

	*pq31_y = (q31_t)(q63_acc >> 32);

	return 0;
}

/************************************************
 *			Oversampler Structure				*
 ************************************************/

void *
oversampler_init(f64_t f64_orig_sample_rate, uint32_t u32_orig_buffer_len, uint32_t u32_ovrsmp_factor)
{
	return 0;
}

int
oversampler_deinit(void *p_ovrsmp)
{
	return 0;
}

int
oversampler_flush(void *p_ovrsmp)
{
	return 0;
}

int
oversampler_get_oversampled_buffers(void *p_ovrsmp, q31_t **pq31_ovrsmp_out, q31_t **pq31_ovrsmp_in)
{
	return 0;
}

int
oversampler_interpolate(void *p_ref, q31_t *pq31_orig_in)
{
	struct OVRSMP *p_ovrsmp = p_ref;

	uint32_t 	u32_i, u32_j;

	q31_t		q31_x, q31_y, *pq31_interp_buffer = NULL;

	if (!p_ovrsmp || !pq31_orig_in)
		return -1;

	if (!(pq31_interp_buffer = p_ovrsmp->pq31_interp_buffer))
		return -1;

	/* For each sample */
	for (u32_i = 0; u32_i < p_ovrsmp->u32_orig_buffer_len; u32_i++) {

		q31_x = *pq31_orig_in++;

		/* For each interpolated sample */
		for (u32_j = 0; u32_j < p_ovrsmp->u32_ovrsmp_factor; u32_j++) {

			if (_convolute(p_ovrsmp->pcrb_fir_hist[u32_j], p_ovrsmp->pq31_filterbank[u32_j], &q31_x, &q31_y))
				return -1;

			*pq31_interp_buffer++ = q31_y;
		}		
	}
	return 0;
}

int
oversampler_decimate(void *p_ovrsmp, q31_t *pq31_orig_out)
{
	return 0;
}

