#include <fixedpoint.h>
#include <iir.h>

/****************************************************************
 *						Plugin definitions						*
 ****************************************************************/

#define BIQUAD_COEFFS 5

/* Staged coefficients offset */
enum {
	STAGE_1 = 0,
	STAGE_2 = 5,
	STAGE_3 = 10,
	STAGE_4 = 15
};

/* Coefficients index */
enum {
	COEFF_B0 = 0,
	COEFF_B1 = 1,
	COEFF_B2 = 2,
	COEFF_A1 = 3,
	COEFF_A2 = 4
};

/* Filter type enum */
enum {
	LPF_6DB = 0, LPF_12DB, LPF_24DB, LPF_36DB, LPF_48DB,
	HPF_6DB, HPF_12DB, HPF_24DB, HPF_36DB, HPF_48DB,
	LSH_1ST, LSH_2ND, HSH_1ST, HSH_2ND,
	PARAM
};

/* Plugin parameter enum */
enum {
	BYPASS = 0,
	FREQ,
	RESO,
	GAIN,
	TYPE
};

/* Plugin instance structure */
struct Filter {
	/* IIR instances */
	struct   IIR *p_iir_left, *p_iir_right;

	/* Filter parameters */
	double   f64_freq, f64_reso, f64_gain;

	/* Filter type id */
	uint32_t filt_type;

	/* Bypass */
	uint32_t u32_bypass;

	/* Sampling frequency */
	double   f64_sampling_freq;
};

/****************************************************************
 *						Static functions						*
 ****************************************************************/

static int
_calculate_LPF_6DB(struct Filter *p_filter)
{
	uint32_t u32_i;
	double   pf64_coeffs[BIQUAD_COEFFS];
	q31_t    pq31_coeffs[BIQUAD_COEFFS];

	double f64_theta_c = 2.0 * M_PI * (p_filter->f64_freq) / (p_filter->f64_sampling_freq);
	double f64_gamma   = cos(f64_theta_c) / (1.0 + sin(f64_theta_c));

	pf64_coeffs[STAGE_1 + COEFF_B0] = (1.0 - f64_gamma) / 2.0;
	pf64_coeffs[STAGE_1 + COEFF_B1] = (1.0 - f64_gamma) / 2.0;
	pf64_coeffs[STAGE_1 + COEFF_B2] = 0.0;
	pf64_coeffs[STAGE_1 + COEFF_A1] = f64_gamma;
	pf64_coeffs[STAGE_1 + COEFF_A2] = 0.0;

	for (u32_i = 0; u32_i < BIQUAD_COEFFS; u32_i++) {
		pf64_coeffs[u32_i] *= 0.25;
		pq31_coeffs[u32_i] = F64_TO_Q31(pf64_coeffs[u32_i]);
	}

	if ((iir_set_stages(p_filter->p_iir_left, 1)) || iir_set_stages(p_filter->p_iir_right, 1))
		return -1;

	if ((iir_set_shift(p_filter->p_iir_left, 2)) || (iir_set_shift(p_filter->p_iir_right, 2)))
		return -1;

	if (iir_set_coeffs(p_filter->p_iir_left, pq31_coeffs, BIQUAD_COEFFS) ||
		iir_set_coeffs(p_filter->p_iir_right, pq31_coeffs, BIQUAD_COEFFS))
		return -1;

	return 0;
}

static int
_calculate_LPF_12DB(struct Filter *p_filter)
{
	uint32_t u32_i;
	double   pf64_coeffs[BIQUAD_COEFFS];
	q31_t    pq31_coeffs[BIQUAD_COEFFS];

	double f64_theta_c = 2.0 * M_PI * (p_filter->f64_freq) / (p_filter->f64_sampling_freq);
	double f64_d       = 1.0 / (p_filter->f64_reso);
	double f64_beta    = 0.5 * ((1.0 - ((f64_d/2.0) * sin(f64_theta_c))) / (1.0 + ((f64_d/2.0) * sin(f64_theta_c))));
	double f64_gamma   = (0.5 + f64_beta) * cos(f64_theta_c);

	pf64_coeffs[STAGE_1 + COEFF_B0] = (0.5 + f64_beta - f64_gamma) / 2.0;
	pf64_coeffs[STAGE_1 + COEFF_B1] = 0.5 + f64_beta - f64_gamma;
	pf64_coeffs[STAGE_1 + COEFF_B2] = (0.5 + f64_beta - f64_gamma) / 2.0;
	pf64_coeffs[STAGE_1 + COEFF_A1] = 2.0 * f64_gamma;
	pf64_coeffs[STAGE_1 + COEFF_A2] = -(2.0 * f64_beta);

	for (u32_i = 0; u32_i < BIQUAD_COEFFS; u32_i++) {
		pf64_coeffs[u32_i] *= 0.25;
		pq31_coeffs[u32_i] = F64_TO_Q31(pf64_coeffs[u32_i]);
	}

	if ((iir_set_stages(p_filter->p_iir_left, 1)) || iir_set_stages(p_filter->p_iir_right, 1))
		return -1;

	if ((iir_set_shift(p_filter->p_iir_left, 2)) || (iir_set_shift(p_filter->p_iir_right, 2)))
		return -1;

	if (iir_set_coeffs(p_filter->p_iir_left, pq31_coeffs, BIQUAD_COEFFS) ||
		iir_set_coeffs(p_filter->p_iir_right, pq31_coeffs, BIQUAD_COEFFS))
		return -1;

	return 0;
}

static int
_calculate_LPF_24DB(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_LPF_36DB(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_LPF_48DB(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_HPF_6DB(struct Filter *p_filter)
{
	uint32_t u32_i;
	double   pf64_coeffs[BIQUAD_COEFFS];
	q31_t    pq31_coeffs[BIQUAD_COEFFS];

	double f64_theta_c = 2.0 * M_PI * (p_filter->f64_freq) / (p_filter->f64_sampling_freq);
	double f64_gamma   = cos(f64_theta_c) / (1.0 + sin(f64_theta_c));

	pf64_coeffs[STAGE_1 + COEFF_B0] = (1.0 + f64_gamma) / 2.0;
	pf64_coeffs[STAGE_1 + COEFF_B1] = -(1.0 + f64_gamma) / 2.0;
	pf64_coeffs[STAGE_1 + COEFF_B2] = 0.0;
	pf64_coeffs[STAGE_1 + COEFF_A1] = f64_gamma;
	pf64_coeffs[STAGE_1 + COEFF_A2] = 0.0;

	for (u32_i = 0; u32_i < BIQUAD_COEFFS; u32_i++) {
		pf64_coeffs[u32_i] *= 0.25;
		pq31_coeffs[u32_i] = F64_TO_Q31(pf64_coeffs[u32_i]);
	}

	if ((iir_set_stages(p_filter->p_iir_left, 1)) || iir_set_stages(p_filter->p_iir_right, 1))
		return -1;

	if ((iir_set_shift(p_filter->p_iir_left, 2)) || (iir_set_shift(p_filter->p_iir_right, 2)))
		return -1;

	if (iir_set_coeffs(p_filter->p_iir_left, pq31_coeffs, BIQUAD_COEFFS) ||
		iir_set_coeffs(p_filter->p_iir_right, pq31_coeffs, BIQUAD_COEFFS))
		return -1;

	return 0;
}

static int
_calculate_HPF_12DB(struct Filter *p_filter)
{
	uint32_t u32_i;
	double   pf64_coeffs[BIQUAD_COEFFS];
	q31_t    pq31_coeffs[BIQUAD_COEFFS];

	double f64_theta_c = 2.0 * M_PI * (p_filter->f64_freq) / (p_filter->f64_sampling_freq);
	double f64_d       = 1.0 / (p_filter->f64_reso);
	double f64_beta    = 0.5 * ((1.0 - (f64_d/2.0) * sin(f64_theta_c)) / (1.0 + (f64_d/2.0) * sin(f64_theta_c)));
	double f64_gamma   = (0.5 + f64_beta) * cos(f64_theta_c);

	pf64_coeffs[STAGE_1 + COEFF_B0] = (0.5 + f64_beta + f64_gamma) / 2.0;
	pf64_coeffs[STAGE_1 + COEFF_B1] = -(0.5 + f64_beta + f64_gamma);
	pf64_coeffs[STAGE_1 + COEFF_B2] = (0.5 + f64_beta + f64_gamma) / 2.0;
	pf64_coeffs[STAGE_1 + COEFF_A1] = 2.0 * f64_gamma;
	pf64_coeffs[STAGE_1 + COEFF_A2] = -(2.0 * f64_beta);

	for (u32_i = 0; u32_i < BIQUAD_COEFFS; u32_i++) {
		pf64_coeffs[u32_i] *= 0.25;
		pq31_coeffs[u32_i] = F64_TO_Q31(pf64_coeffs[u32_i]);
	}

	if ((iir_set_stages(p_filter->p_iir_left, 1)) || iir_set_stages(p_filter->p_iir_right, 1))
		return -1;

	if ((iir_set_shift(p_filter->p_iir_left, 2)) || (iir_set_shift(p_filter->p_iir_right, 2)))
		return -1;

	if (iir_set_coeffs(p_filter->p_iir_left, pq31_coeffs, BIQUAD_COEFFS) ||
		iir_set_coeffs(p_filter->p_iir_right, pq31_coeffs, BIQUAD_COEFFS))
		return -1;

	return 0;
}

static int
_calculate_HPF_24DB(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_HPF_36DB(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_HPF_48DB(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_LSH_1ST(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_LSH_2ND(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_HSH_1ST(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_HSH_2ND(struct Filter *p_filter)
{
	(void) p_filter;
	return 0;
}

static int
_calculate_PARAM(struct Filter *p_filter)
{
	uint32_t u32_i;
	double   f64_den;
	double   pf64_coeffs[BIQUAD_COEFFS];
	q31_t    pq31_coeffs[BIQUAD_COEFFS];

	double f64_theta     = 2.0 * M_PI * (p_filter->f64_freq) / (p_filter->f64_sampling_freq);
	double f64_K         = tan(f64_theta / 2.0);
	double f64_W         = f64_K * f64_K;
	double f64_norm_gain = exp((p_filter->f64_gain) * 0.115129254);

	if (f64_norm_gain < 1.0) {
		f64_den = 1.0 + (f64_K / (f64_norm_gain * (p_filter->f64_reso))) + f64_W;

		pf64_coeffs[STAGE_1 + COEFF_B0] = (1.0 + (f64_K / (p_filter->f64_reso)) + f64_W) / f64_den;
		pf64_coeffs[STAGE_1 + COEFF_B1] = 2.0 * (f64_W - 1.0) / f64_den;
		pf64_coeffs[STAGE_1 + COEFF_B2] = (1.0 - (f64_K / (p_filter->f64_reso)) + f64_W) / f64_den;
		pf64_coeffs[STAGE_1 + COEFF_A1] = -2.0 * (f64_W - 1.0) / f64_den;
		pf64_coeffs[STAGE_1 + COEFF_A2] = -(1.0 - (f64_K / (f64_norm_gain * (p_filter->f64_reso))) + f64_W) / f64_den;
	} else {
		f64_den = 1.0 + (f64_K / (p_filter->f64_reso)) + f64_W;

		pf64_coeffs[STAGE_1 + COEFF_B0] = (1.0 + ((f64_K * f64_norm_gain) / (p_filter->f64_reso)) + f64_W) / f64_den;
		pf64_coeffs[STAGE_1 + COEFF_B1] = 2.0 * (f64_W - 1.0) / f64_den;
		pf64_coeffs[STAGE_1 + COEFF_B2] = (1.0 - ((f64_K * f64_norm_gain) / (p_filter->f64_reso)) + f64_W) / f64_den;
		pf64_coeffs[STAGE_1 + COEFF_A1] = -2.0 * (f64_W - 1.0) / f64_den;
		pf64_coeffs[STAGE_1 + COEFF_A2] = -(1.0 - (f64_K / (p_filter->f64_reso)) + f64_W) / f64_den;
	}

	for (u32_i = 0; u32_i < BIQUAD_COEFFS; u32_i++) {
		pf64_coeffs[u32_i] *= 0.25;
		pq31_coeffs[u32_i] = F64_TO_Q31(pf64_coeffs[u32_i]);
	}

	if ((iir_set_stages(p_filter->p_iir_left, 1)) || iir_set_stages(p_filter->p_iir_right, 1))
		return -1;

	if ((iir_set_shift(p_filter->p_iir_left, 2)) || (iir_set_shift(p_filter->p_iir_right, 2)))
		return -1;

	if (iir_set_coeffs(p_filter->p_iir_left, pq31_coeffs, BIQUAD_COEFFS) ||
		iir_set_coeffs(p_filter->p_iir_right, pq31_coeffs, BIQUAD_COEFFS))
		return -1;

	return 0;
}

static int
_update_param(struct Filter *p_filter)
{
	if ((iir_flush(p_filter->p_iir_left)) || (iir_flush(p_filter->p_iir_right)))
		return -1;

	switch (p_filter->filt_type) {
		case LPF_6DB:
			return _calculate_LPF_6DB(p_filter);
		case LPF_12DB:
			return _calculate_LPF_12DB(p_filter);
		case LPF_24DB:
			return _calculate_LPF_24DB(p_filter);
		case LPF_36DB:
			return _calculate_LPF_36DB(p_filter);
		case LPF_48DB:
			return _calculate_LPF_48DB(p_filter);
		case HPF_6DB:
			return _calculate_HPF_6DB(p_filter);
		case HPF_12DB:
			return _calculate_HPF_12DB(p_filter);
		case HPF_24DB:
			return _calculate_HPF_24DB(p_filter);
		case HPF_36DB:
			return _calculate_HPF_36DB(p_filter);
		case HPF_48DB:
			return _calculate_HPF_48DB(p_filter);
		case LSH_1ST:
			return _calculate_LSH_1ST(p_filter);
		case LSH_2ND:
			return _calculate_LSH_2ND(p_filter);
		case HSH_1ST:
			return _calculate_HSH_1ST(p_filter);
		case HSH_2ND:
			return _calculate_HSH_2ND(p_filter);
		case PARAM:
			return _calculate_PARAM(p_filter);
		default:
			return -1;
	}
}

static void
_deinterleave(q31_t *pq31_interleaved,
			  q31_t *pq31_left, q31_t *pq31_right,
			  uint32_t u32_buffer_size)
{
	do {
		*pq31_left++  = *pq31_interleaved++;
		*pq31_right++ = *pq31_interleaved++;
	} while (--u32_buffer_size);
}

static void
_interleave(q31_t *pq31_interleaved,
			q31_t *pq31_left, q31_t *pq31_right,
			uint32_t u32_buffer_size)
{
	do {
		*pq31_interleaved++ = *pq31_left++;
		*pq31_interleaved++ = *pq31_right++;
	} while (--u32_buffer_size);
}

static void
_mono_to_stereo(q31_t *pq31_mono,
				q31_t *pq31_left, q31_t *pq31_right,
				uint32_t u32_buffer_size)
{
	do {
		*pq31_left++  = *pq31_mono;
		*pq31_right++ = *pq31_mono++;
	} while (--u32_buffer_size);
}

/****************************************************************
 *						Public functions						*
 ****************************************************************/

void *
plugin_init(double f64_sampling_freq)
{
	struct Filter *p_filter = calloc(1, sizeof(struct Filter));

	if (!p_filter)
		return NULL;

	if (f64_sampling_freq < 0)
		goto cleanup;

	p_filter->f64_sampling_freq = f64_sampling_freq;

	if (!(p_filter->p_iir_left = iir_init()) || !(p_filter->p_iir_right = iir_init()))
		goto cleanup;

	return p_filter;

cleanup:

	iir_deinit(p_filter->p_iir_left);
	iir_deinit(p_filter->p_iir_right);

	free(p_filter);

	return NULL;
}

int
plugin_deinit(void *p_plugin)
{
	struct Filter *p_filter = p_plugin;

	if (!p_filter)
		return -1;

	iir_deinit(p_filter->p_iir_left);
	iir_deinit(p_filter->p_iir_right);

	free(p_filter);

	return 0;
}

int
plugin_set_param(void *p_plugin, int param, void *p_value)
{
	struct Filter *p_filter = p_plugin;

	if (!p_filter || !p_value)
		return -1;

	switch (param) {
		case BYPASS:
			if ((*((uint32_t *)(p_value)) != 1) && (*((uint32_t *)(p_value)) != 0))
				return -1;

			memcpy(&(p_filter->u32_bypass), p_value, sizeof(uint32_t));
			break;

		case FREQ:
			if (*((double *)(p_value)) < 0.0)
				return -1;

			memcpy(&(p_filter->f64_freq), p_value, sizeof(double));
			break;

		case RESO:
			if (*((double *)(p_value)) < 0.0)
				return -1;

			memcpy(&(p_filter->f64_reso), p_value, sizeof(double));
			break;

		case GAIN:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_filter->f64_gain), p_value, sizeof(double));
			break;

		case TYPE:
			if (*((uint32_t *)(p_value)) > PARAM)
				return -1;

			memcpy(&(p_filter->filt_type), p_value, sizeof(uint32_t));
			break;

		default:
			return -1;
	}

	return _update_param(p_filter);
}

int
plugin_get_param(void *p_plugin, int param, void *p_value)
{
	struct Filter *p_filter = p_plugin;

	if (!p_filter || !p_value)
		return -1;

	switch (param) {
		case BYPASS:
			memcpy(p_value, &(p_filter->u32_bypass), sizeof(uint32_t));
			break;

		case FREQ:
			memcpy(p_value, &(p_filter->f64_freq), sizeof(double));
			break;

		case RESO:
			memcpy(p_value, &(p_filter->f64_reso), sizeof(double));
			break;

		case GAIN:
			memcpy(p_value, &(p_filter->f64_gain), sizeof(double));
			break;

		case TYPE:
			memcpy(p_value, &(p_filter->filt_type), sizeof(int));
			break;

		default:
			return -1;
	}

	return 0;
}

int
plugin_process__MONO_IN_MONO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct Filter *p_filter = p_plugin;

	if (!p_filter || !pq31_src || !pq31_dst)
		return -1;

	if (p_filter->u32_bypass) {
		memcpy(pq31_dst, pq31_src, u32_buffer_size * sizeof(q31_t));
		return 0;
	}

	if (iir_do(p_filter->p_iir_left, pq31_src, pq31_dst, u32_buffer_size))
		return -1;

	return 0;
}

int
plugin_process__MONO_IN_STEREO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct Filter *p_filter = p_plugin;

	if (!p_filter || !pq31_src || !pq31_dst)
		return -1;

	if (p_filter->u32_bypass) {
		do {
			*pq31_dst++ = *pq31_src;
			*pq31_dst++ = *pq31_src++;
		} while (--u32_buffer_size);
		return 0;
	}

	q31_t pq31_src_left[u32_buffer_size];
	q31_t pq31_src_right[u32_buffer_size];
	q31_t pq31_dst_left[u32_buffer_size];
	q31_t pq31_dst_right[u32_buffer_size];

	_mono_to_stereo(pq31_src, pq31_src_left, pq31_src_right, u32_buffer_size);

	if (iir_do(p_filter->p_iir_left, pq31_src_left, pq31_dst_left, u32_buffer_size))
		return -1;

	if (iir_do(p_filter->p_iir_right, pq31_src_right, pq31_dst_right, u32_buffer_size))
		return -1;

	_interleave(pq31_dst, pq31_dst_left, pq31_dst_right, u32_buffer_size);

	return 0;
}

int
plugin_process__STEREO_IN_STEREO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct Filter *p_filter = p_plugin;

	if (!p_filter || !pq31_src || !pq31_dst)
		return -1;

	if (p_filter->u32_bypass) {
		memcpy(pq31_dst, pq31_src, 2 * u32_buffer_size * sizeof(q31_t));
		return 0;
	}

	q31_t pq31_src_left[u32_buffer_size];
	q31_t pq31_src_right[u32_buffer_size];
	q31_t pq31_dst_left[u32_buffer_size];
	q31_t pq31_dst_right[u32_buffer_size];

	_deinterleave(pq31_src, pq31_src_left, pq31_src_right, u32_buffer_size);

	if (iir_do(p_filter->p_iir_left, pq31_src_left, pq31_dst_left, u32_buffer_size))
		return -1;

	if (iir_do(p_filter->p_iir_right, pq31_src_right, pq31_dst_right, u32_buffer_size))
		return -1;

	_interleave(pq31_dst, pq31_dst_left, pq31_dst_right, u32_buffer_size);

	return 0;
}
