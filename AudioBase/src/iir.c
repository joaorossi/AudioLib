#include <iir.h>

/************************************************
 *					Definitions					*
 ************************************************/

#define BIQUAD_COEFFS       5
#define BIQUAD_STATES       4
#define MAX_CASCADED_STAGES 4
#define Q63_TO_Q31_SHIFT    32

/************************************************
 *				Filter Structure				*
 ************************************************/

/* IIR Filter structure */
struct IIR {
	/* Coefficients vector */
	q31_t    *pq31_coeffs;

	/* Internal state vector */
	q31_t    *pq31_state;

	/* Post MAC shift */
	uint32_t u32_shift;

	/* Nunber of biquad stages */
	uint32_t u32_stages;
};

/************************************************
 *		32 Bits States IIR Implamentation		*
 ************************************************/

void *
iir_init()
{
	struct IIR *p_iir = calloc(1, sizeof(struct IIR));

	if (!p_iir)
		return NULL;

	p_iir->pq31_coeffs = calloc(1, MAX_CASCADED_STAGES * BIQUAD_COEFFS * sizeof(q31_t));
	p_iir->pq31_state  = calloc(1, MAX_CASCADED_STAGES * BIQUAD_STATES * sizeof(q31_t));

	if (!p_iir->pq31_state || !p_iir->pq31_coeffs)
		goto cleanup;

	p_iir->u32_stages = 1;

	return p_iir;

cleanup:

	if (p_iir->pq31_coeffs)
		free(p_iir->pq31_coeffs);

	if (p_iir->pq31_state)
		free(p_iir->pq31_state);

	free(p_iir);

	return NULL;
}

int
iir_deinit(void *p_handle)
{
	struct IIR *p_iir = p_handle;

	if (!p_iir)
		return -1;

	if (p_iir->pq31_coeffs)
		free(p_iir->pq31_coeffs);

	if (p_iir->pq31_state)
		free(p_iir->pq31_state);

	free(p_iir);

	return 0;
}

int
iir_flush(void *p_handle)
{
	struct IIR *p_iir = p_handle;

	if (!p_iir)
		return -1;

	if (!p_iir->pq31_coeffs || !p_iir->pq31_state)
		return -1;

	memset(p_iir->pq31_coeffs, 0, MAX_CASCADED_STAGES * BIQUAD_COEFFS * sizeof(q31_t));
	memset(p_iir->pq31_state,  0, MAX_CASCADED_STAGES * BIQUAD_STATES * sizeof(q31_t));

	return 0;
}

int
iir_set_coeffs(void *p_handle, q31_t *pq31_coeffs, uint32_t u32_n)
{
	struct IIR *p_iir = p_handle;

	if (!p_iir || !pq31_coeffs)
		return -1;

	if (!p_iir->pq31_coeffs)
		return -1;

	if ((u32_n != (p_iir->u32_stages * BIQUAD_COEFFS)) && (u32_n > (MAX_CASCADED_STAGES * BIQUAD_COEFFS)))
		return -1;

	memcpy(p_iir->pq31_coeffs, pq31_coeffs, u32_n * sizeof(q31_t));

	return 0;
}

int
iir_set_stages(void *p_handle, uint32_t u32_n)
{
	struct IIR *p_iir = p_handle;

	if (!p_iir)
		return -1;

	if (u32_n > MAX_CASCADED_STAGES)
		return -1;

	p_iir->u32_stages = u32_n;

	return 0;
}

int
iir_set_shift(void *p_handle, uint32_t u32_n)
{
	struct IIR *p_iir = p_handle;

	if (!p_iir)
		return -1;

	if (u32_n > 16)
		return -1;

	p_iir->u32_shift = u32_n;

	return 0;
}

int
iir_do(void *p_handle, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct IIR *p_iir = p_handle;

	if (!p_iir || !pq31_src || !pq31_dst)
		return -1;

	if (!p_iir->pq31_coeffs || !p_iir->pq31_state)
		return -1;

	q31_t *pq31_coeffs = p_iir->pq31_coeffs;
	q31_t *pq31_state  = p_iir->pq31_state;

	uint32_t u32_shift = p_iir->u32_shift + 1;
	uint32_t u32_stages = p_iir->u32_stages;
	uint32_t u32_samples;

	q31_t q31_b0, q31_b1, q31_b2, q31_a1, q31_a2;
	q31_t q31_xn, q31_x1, q31_x2, q31_y1, q31_y2;
	q63_t q63_acc;

	do {
		q31_b0 = *pq31_coeffs++;
		q31_b1 = *pq31_coeffs++;
		q31_b2 = *pq31_coeffs++;
		q31_a1 = *pq31_coeffs++;
		q31_a2 = *pq31_coeffs++;

		q31_x1 = pq31_state[0];
		q31_x2 = pq31_state[1];
		q31_y1 = pq31_state[2];
		q31_y2 = pq31_state[3];

		u32_samples = u32_buffer_size;

		do {
			q31_xn = *pq31_src++;

			q63_acc = 0;

			MAC__Q63_Q31_Q31(q63_acc, q31_xn, q31_b0);
			MAC__Q63_Q31_Q31(q63_acc, q31_x1, q31_b1);
			MAC__Q63_Q31_Q31(q63_acc, q31_x2, q31_b2);
			MAC__Q63_Q31_Q31(q63_acc, q31_y1, q31_a1);
			MAC__Q63_Q31_Q31(q63_acc, q31_y2, q31_a2);

			q31_x2 = q31_x1;
			q31_x1 = q31_xn;
			q31_y2 = q31_y1;
			q31_y1 = (q31_t)((q63_acc << u32_shift) >> Q63_TO_Q31_SHIFT);

			*pq31_dst++ = q31_y1;
		} while (--u32_samples);

		*pq31_state++ = q31_x1;
		*pq31_state++ = q31_x2;
		*pq31_state++ = q31_y1;
		*pq31_state++ = q31_y2;
	} while (--u32_stages);

	return 0;
}
