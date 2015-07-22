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

	/* Polyphase filterbank */
	uint32_t	u32_phase_ir_len;
	q31_t		**ppq31_filterbank;

	/* Convolution ring buffers */
	struct CRB	**ppcrb_interp_hist;
	struct CRB	**ppcrb_deci_hist;

	/* Oversampled buffer */
	uint32_t	u32_ovrsmp_buff_len;
	q31_t		*pq31_interp_buffer;
	q31_t		*pq31_deci_buffer;

};

/************************************************
 *			Static helper functions				*
 ************************************************/

static int
_fill_filterbank(f64_t f64_orig_sample_rate, uint32_t u32_ovrsmp_factor,
				 q31_t **ppq31_filterbank, uint32_t *u32_phase_ir_len)
{
	return 0;
}

static struct CRB *
_crb_init(uint32_t u32_buff_len)
{
	struct CRB *pcrb = NULL;
	
	if (!(pcrb = calloc(1, sizeof(struct CRB))))
		goto error;

	if (!(pcrb->pq31_ring_buffer = malloc(u32_buff_len * sizeof(q31_t))))
		goto error;

	pcrb->u32_buff_len = u32_buff_len;

	return pcrb;

error:

	if (pcrb) {
		if (pcrb->pq31_ring_buffer)
			free(pcrb->pq31_ring_buffer);

		free(pcrb);
	}

	return NULL;
}

static int
_crb_deinit(struct CRB *pcrb)
{
	if (!pcrb)
		return -1;

	if (pcrb->pq31_ring_buffer)
		free(pcrb->pq31_ring_buffer);

	free(pcrb);

	return 0;
}

static int
_crb_flush(struct CRB *pcrb)
{
	if (!pcrb)
		return -1;

	if (!pcrb->pq31_ring_buffer)
		return -1;

	memset(pcrb->pq31_ring_buffer, 0, pcrb->u32_buff_len * sizeof(q31_t));

	return 0;
}

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
 *			Oversampler Public Functions		*
 ************************************************/

void *
oversampler_init(f64_t f64_orig_sample_rate, uint32_t u32_orig_buffer_len, uint32_t u32_ovrsmp_factor)
{
	struct OVRSMP *p_ovrsmp = NULL;
	uint32_t u32_i = 0;

	/* Check sample rate */
	if ((f64_orig_sample_rate == 44100.0) ||
		(f64_orig_sample_rate == 48000.0) ||
		(f64_orig_sample_rate == 88200.0) ||
		(f64_orig_sample_rate == 96000.0))
		return NULL;

	/* Check buffer length */
	if (u32_orig_buffer_len > MAX_OVRSMP_ORIG_BUFF_LEN)
		return NULL;

	/* Check oversample factor */
	if ((u32_ovrsmp_factor == 2) ||
		(u32_ovrsmp_factor == 4) ||
		(u32_ovrsmp_factor == 8))
		return NULL;

	/* Alloc oversampler instance */
	if (!(p_ovrsmp = calloc(1, sizeof(struct OVRSMP))))
		return NULL;

	/* Fill structure with checked data */
	p_ovrsmp->f64_orig_sample_rate = f64_orig_sample_rate;
	p_ovrsmp->f64_ovrsmp_rate = f64_orig_sample_rate * u32_ovrsmp_factor;
	p_ovrsmp->u32_orig_buffer_len = u32_orig_buffer_len;
	p_ovrsmp->u32_ovrsmp_factor = u32_ovrsmp_factor;

	/* Fill filterbank with IRs */
	if (!(p_ovrsmp->ppq31_filterbank = calloc(p_ovrsmp->u32_ovrsmp_factor, sizeof(q31_t *))))
		goto cleanup;

	if (_fill_filterbank(p_ovrsmp->f64_orig_sample_rate, p_ovrsmp->u32_ovrsmp_factor,
						 p_ovrsmp->ppq31_filterbank, &(p_ovrsmp->u32_phase_ir_len)))
		goto cleanup;

	/* Alloc convolution ring buffers */
	if (!(p_ovrsmp->ppcrb_interp_hist = calloc(p_ovrsmp->u32_ovrsmp_factor, sizeof(struct CRB *))) ||
		!(p_ovrsmp->ppcrb_deci_hist   = calloc(p_ovrsmp->u32_ovrsmp_factor, sizeof(struct CRB *))))
		goto cleanup;

	for (u32_i = 0; u32_i < p_ovrsmp->u32_ovrsmp_factor; u32_i++) {
		if (!(p_ovrsmp->ppcrb_interp_hist[u32_i] = _crb_init(p_ovrsmp->u32_phase_ir_len)) ||
			!(p_ovrsmp->ppcrb_deci_hist[u32_i]   = _crb_init(p_ovrsmp->u32_phase_ir_len)))
			goto cleanup;
	}

	/* Alloc oversampled buffers */
	p_ovrsmp->u32_ovrsmp_buff_len = u32_orig_buffer_len * u32_ovrsmp_factor;

	if (!(p_ovrsmp->pq31_interp_buffer = calloc(1, p_ovrsmp->u32_ovrsmp_buff_len * sizeof(q31_t))) ||
		!(p_ovrsmp->pq31_deci_buffer   = calloc(1, p_ovrsmp->u32_ovrsmp_buff_len * sizeof(q31_t))))
		goto cleanup;

	return p_ovrsmp;

cleanup:

	if (p_ovrsmp->ppq31_filterbank) {

		for (u32_i = 0; u32_i < p_ovrsmp->u32_ovrsmp_factor; u32_i++) {
			if (p_ovrsmp->ppq31_filterbank[u32_i])
				free(p_ovrsmp->ppq31_filterbank[u32_i]);
		}
		free(p_ovrsmp->ppq31_filterbank);
	}

	if (p_ovrsmp->ppcrb_interp_hist) {
		for (u32_i = 0; u32_i < p_ovrsmp->u32_ovrsmp_factor; u32_i++)
			_crb_deinit(p_ovrsmp->ppcrb_interp_hist[u32_i]);

		free(p_ovrsmp->ppcrb_interp_hist);
	}

	if (p_ovrsmp->ppcrb_deci_hist) {
		for (u32_i = 0; u32_i < p_ovrsmp->u32_ovrsmp_factor; u32_i++)
			_crb_deinit(p_ovrsmp->ppcrb_deci_hist[u32_i]);

		free(p_ovrsmp->ppcrb_deci_hist);
	}

	if (p_ovrsmp->pq31_interp_buffer)
		free(p_ovrsmp->pq31_interp_buffer);

	if (p_ovrsmp->pq31_deci_buffer)
			free(p_ovrsmp->pq31_deci_buffer);

	return NULL;
}

int
oversampler_deinit(void *p_ref)
{
	struct OVRSMP *p_ovrsmp = p_ref;
	uint32_t u32_i;

	if (!p_ovrsmp)
		return -1;

	if (p_ovrsmp->ppq31_filterbank) {

		for (u32_i = 0; u32_i < p_ovrsmp->u32_ovrsmp_factor; u32_i++) {
			if (p_ovrsmp->ppq31_filterbank[u32_i])
				free(p_ovrsmp->ppq31_filterbank[u32_i]);
		}
		free(p_ovrsmp->ppq31_filterbank);
	}

	if (p_ovrsmp->ppcrb_interp_hist) {
		for (u32_i = 0; u32_i < p_ovrsmp->u32_ovrsmp_factor; u32_i++)
			_crb_deinit(p_ovrsmp->ppcrb_interp_hist[u32_i]);

		free(p_ovrsmp->ppcrb_interp_hist);
	}

	if (p_ovrsmp->ppcrb_deci_hist) {
		for (u32_i = 0; u32_i < p_ovrsmp->u32_ovrsmp_factor; u32_i++)
			_crb_deinit(p_ovrsmp->ppcrb_deci_hist[u32_i]);

		free(p_ovrsmp->ppcrb_deci_hist);
	}

	if (p_ovrsmp->pq31_interp_buffer)
		free(p_ovrsmp->pq31_interp_buffer);

	if (p_ovrsmp->pq31_deci_buffer)
			free(p_ovrsmp->pq31_deci_buffer);

	return 0;
}

int
oversampler_flush(void *p_ref)
{
	struct OVRSMP *p_ovrsmp = p_ref;
	uint32_t u32_i;

	if (!p_ovrsmp)
		return -1;

	/* Flush ring buffers */
	for (u32_i = 0; u32_i < p_ovrsmp->u32_ovrsmp_factor; u32_i++) {

		if (_crb_flush(p_ovrsmp->ppcrb_interp_hist[u32_i]) ||
			_crb_flush(p_ovrsmp->ppcrb_deci_hist[u32_i]))
			return -1;
	}

	/* Flush oversample buffers */
	memset(p_ovrsmp->pq31_interp_buffer, 0, p_ovrsmp->u32_ovrsmp_buff_len * sizeof(q31_t));
	memset(p_ovrsmp->pq31_deci_buffer, 0, p_ovrsmp->u32_ovrsmp_buff_len * sizeof(q31_t));

	return 0;
}

int
oversampler_get_oversampled_buffers(void *p_ref, q31_t **pq31_ovrsmp_out, q31_t **pq31_ovrsmp_in)
{
	struct OVRSMP *p_ovrsmp = p_ref;

	if (!p_ovrsmp)
		return -1;

	*pq31_ovrsmp_out = p_ovrsmp->pq31_interp_buffer;
	*pq31_ovrsmp_in  = p_ovrsmp->pq31_deci_buffer;

	return 0;
}

int
oversampler_interpolate(void *p_ref, q31_t *pq31_orig_in)
{
	struct OVRSMP *p_ovrsmp = p_ref;
	uint32_t u32_i, u32_j;
	q31_t q31_x, q31_y, *pq31_interp_buffer;

	if (!p_ovrsmp || !pq31_orig_in)
		return -1;

	if (!(pq31_interp_buffer = p_ovrsmp->pq31_interp_buffer))
		return -1;

	/* For each input sample */
	for (u32_i = 0; u32_i < p_ovrsmp->u32_orig_buffer_len; u32_i++) {

		q31_x = *pq31_orig_in++;

		/* For each interpolated sample */
		for (u32_j = 0; u32_j < p_ovrsmp->u32_ovrsmp_factor; u32_j++) {

			if (_convolute(p_ovrsmp->ppcrb_interp_hist[u32_j], p_ovrsmp->ppq31_filterbank[u32_j], &q31_x, &q31_y))
				return -1;

			*pq31_interp_buffer++ = q31_y;
		}		
	}
	return 0;
}

int
oversampler_decimate(void *p_ref, q31_t *pq31_orig_out)
{
	struct OVRSMP *p_ovrsmp = p_ref;
	uint32_t u32_i, u32_j;
	q31_t q31_y, q31_x, q31_acc, *pq31_deci_buffer;

	if (!p_ovrsmp || !pq31_orig_out)
		return -1;

	if (!(pq31_deci_buffer = p_ovrsmp->pq31_deci_buffer))
		return -1;

	/* For each output sample */
	for (u32_i = 0; u32_i < p_ovrsmp->u32_orig_buffer_len; u32_i++) {

		q31_acc = 0;

		/* For each interpolated sample */
		for (u32_j = 0; u32_j < p_ovrsmp->u32_orig_buffer_len; u32_j++) {

			q31_x = *pq31_deci_buffer++;

			if (_convolute(p_ovrsmp->ppcrb_deci_hist[u32_j], p_ovrsmp->ppq31_filterbank[u32_j], &q31_x, &q31_y))
				return -1;

			q31_acc += q31_y;
		}

		*pq31_orig_out++ = q31_acc;
	}

	return 0;
}

