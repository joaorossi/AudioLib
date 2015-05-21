#include <fixedpoint.h>
#include <delay_line.h>
/*#include <dlfcn.h>*/

/****************************************************************
 *						Plugin definitions						*
 ****************************************************************/

/*#define AUDIOBASE_PATH "/Users/joaorossifilho/Documents/Cronus/Labs/Z-Core/AudioBase/bin/libAudioBase.dylib"*/

#define MAX_DELAY_LENGTH 192000

/* Parameters enum */
enum {
	BYPASS = 0,
	TIME,
	FEEDBACK,
	MIX
};

/* Plugin instance structure */
struct Delay {
	/* Delay line instances */
	void     *p_delay_left, *p_delay_right;

	/* COntrol parameters */
	double   f64_delay_time, f64_feedback_pct, f64_mix_pct;

	/* Internal fixed point parameters */
	q31_t    q31_feedback, q31_dry, q31_wet;
	uint32_t u32_delay;

	/* Save delay line output between calls */
	q31_t    q31_out_left, q31_out_right;

	/* Bypass effect */
	uint32_t u32_bypass;

	/* Sampling frequency */
	double   f64_sampling_freq;
};

/****************************************************************
 *						Public functions						*
 ****************************************************************/

void *
plugin_init(double f64_sampling_freq)
{
	struct Delay *p_delay = calloc(1, sizeof(struct Delay));

	if (!p_delay)
		return NULL;

	if (f64_sampling_freq <= 0.0)
		goto cleanup;

	/* Allocate delay line instances */
	if (!(p_delay->p_delay_left  = delay_line_init(MAX_DELAY_LENGTH)) ||
		!(p_delay->p_delay_right = delay_line_init(MAX_DELAY_LENGTH)))
		goto cleanup;

	/* Set internal parameters */
	p_delay->q31_dry           = 0x8fffffff;
	p_delay->f64_sampling_freq = f64_sampling_freq;

	return p_delay;

cleanup:

	delay_line_deinit(p_delay->p_delay_left);
	delay_line_deinit(p_delay->p_delay_right);

	free(p_delay);

	return NULL;
}

int
plugin_deinit(void *p_plugin)
{
	struct Delay *p_delay = p_plugin;
	int ret = 0;

	/*if (!p_delay || !p_delay_line_handle)*/
	if (!p_delay)
		return -1;

	if (delay_line_deinit(p_delay->p_delay_left) ||
		delay_line_deinit(p_delay->p_delay_right))
		ret = -1;

	free(p_delay);

	return ret;
}

int
plugin_set_param(void *p_plugin, int param_id, void *p_value)
{
	struct Delay *p_delay = p_plugin;
	double f64_aux;

	if (!p_delay || !p_value)
		return -1;

	switch (param_id) {
		case BYPASS:
			if ((*((uint32_t *)(p_value)) != 1) && (*((uint32_t *)(p_value)) != 0))
				return -1;

			if (p_delay->u32_bypass == *((uint32_t *)(p_value)))
				return 0;

			p_delay->q31_out_left = 0;
			p_delay->q31_out_right = 0;

			if (delay_line_flush(p_delay->p_delay_left) ||
				delay_line_flush(p_delay->p_delay_right))
				return -1;

			memcpy(&(p_delay->u32_bypass), p_value, sizeof(uint32_t));
			break;

		case TIME:
			if ((*((double *)(p_value)) > (MAX_DELAY_LENGTH/p_delay->f64_sampling_freq)) || (*((double *)(p_value)) < 0.0))
				return -1;

			memcpy(&(p_delay->f64_delay_time), p_value, sizeof(double));
			p_delay->u32_delay = lrint(p_delay->f64_delay_time * p_delay->f64_sampling_freq);

			if (delay_line_set_delay(p_delay->p_delay_left, p_delay->u32_delay) ||
				delay_line_set_delay(p_delay->p_delay_right, p_delay->u32_delay))
				return -1;
			break;

		case FEEDBACK:
			if ((*((double *)(p_value)) > 100.0) || (*((double *)(p_value)) < 0.0))
				return -1;

			memcpy(&(p_delay->f64_feedback_pct), p_value, sizeof(double));

			f64_aux = p_delay->f64_feedback_pct/100.0;
			p_delay->q31_feedback = F64_TO_Q31(f64_aux);
			break;

		case MIX:
			if ((*((double *)(p_value)) > 100.0) || (*((double *)(p_value)) < 0.0))
				return -1;

			memcpy(&(p_delay->f64_mix_pct), p_value, sizeof(double));

			f64_aux = p_delay->f64_mix_pct/100.0;
			p_delay->q31_wet = F64_TO_Q31(f64_aux);

			f64_aux = 1.0 - (p_delay->f64_mix_pct/100.0);
			p_delay->q31_dry = F64_TO_Q31(f64_aux);
			break;

		default:
			return -1;
	}

	return 0;
}

int
plugin_get_param(void *p_plugin, int param_id, void *p_value)
{
	struct Delay *p_delay = p_plugin;

	if (!p_delay || !p_value)
		return -1;

	switch (param_id) {
		case BYPASS:
			memcpy(p_value, &(p_delay->u32_bypass), sizeof(uint32_t));
			break;

		case TIME:
			memcpy(p_value, &(p_delay->f64_delay_time), sizeof(double));
			break;

		case FEEDBACK:
			memcpy(p_value, &(p_delay->f64_feedback_pct), sizeof(double));
			break;

		case MIX:
			memcpy(p_value, &(p_delay->f64_mix_pct), sizeof(double));
			break;

		default:
			return -1;
	}

	return 0;
}

int
plugin_process__MONO_IN_MONO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct Delay *p_delay = p_plugin;

	q31_t q31_fdb, q31_in, q31_out;

	if (!p_delay || !pq31_src || !pq31_dst)
		return -1;

	if (p_delay->u32_bypass) {
		memcpy(pq31_dst, pq31_src, u32_buffer_size * sizeof(q31_t));
		return 0;
	}

	q31_out = p_delay->q31_out_left;

	do {
		q31_in = *pq31_src++;

		q31_fdb = q31_in + MUL__Q31(q31_out, p_delay->q31_feedback);

		if (delay_line_do(p_delay->p_delay_left, &q31_fdb, &q31_out))
			return -1;

		*pq31_dst++ = MUL__Q31(q31_in, p_delay->q31_dry) + MUL__Q31(q31_out, p_delay->q31_wet);
	} while (--u32_buffer_size);

	p_delay->q31_out_left = q31_out;

	return 0;
}

int
plugin_process__MONO_IN_STEREO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct Delay *p_delay = p_plugin;

	q31_t q31_in, q31_out_left, q31_out_right, q31_fdb_left, q31_fdb_right;

	if (!p_delay || !pq31_src || !pq31_dst)
		return -1;

	if (p_delay->u32_bypass) {
		do {
			*pq31_dst++ = *pq31_src;
			*pq31_dst++ = *pq31_src++;
		} while (--u32_buffer_size);
		return 0;
	}

	q31_out_left  = p_delay->q31_out_left;
	q31_out_right = p_delay->q31_out_right;

	do {
		q31_in = *pq31_src++;

		q31_fdb_left  = q31_in + MUL__Q31(q31_out_left,  p_delay->q31_feedback);
		q31_fdb_right = q31_in + MUL__Q31(q31_out_right, p_delay->q31_feedback);

		if (delay_line_do(p_delay->p_delay_left,  &q31_fdb_left,  &q31_out_left) ||
			delay_line_do(p_delay->p_delay_right, &q31_fdb_right, &q31_out_right))
			return -1;

		*pq31_dst++ = MUL__Q31(q31_in, p_delay->q31_dry) + MUL__Q31(q31_out_left,  p_delay->q31_wet);
		*pq31_dst++ = MUL__Q31(q31_in, p_delay->q31_dry) + MUL__Q31(q31_out_right, p_delay->q31_wet);
	} while (--u32_buffer_size);

	p_delay->q31_out_left  = q31_out_left;
	p_delay->q31_out_right = q31_out_right;

	return 0;
}

int
plugin_process__STEREO_IN_STEREO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct Delay *p_delay = p_plugin;

	q31_t q31_in_left,  q31_out_left,  q31_fdb_left,
		  q31_in_right, q31_out_right, q31_fdb_right;

	if (!p_delay || !pq31_src || !pq31_dst)
		return -1;

	if (p_delay->u32_bypass) {
		memcpy(pq31_dst, pq31_src, 2 * u32_buffer_size * sizeof(q31_t));
		return 0;
	}

	q31_out_left  = p_delay->q31_out_left;
	q31_out_right = p_delay->q31_out_right;

	do {
		q31_in_left  = *pq31_src++;
		q31_in_right = *pq31_src++;

		q31_fdb_left  = q31_in_left  + MUL__Q31(q31_out_left,  p_delay->q31_feedback);
		q31_fdb_right = q31_in_right + MUL__Q31(q31_out_right, p_delay->q31_feedback);

		if (delay_line_do(p_delay->p_delay_left,  &q31_fdb_left,  &q31_out_left) ||
			delay_line_do(p_delay->p_delay_right, &q31_fdb_right, &q31_out_right))
			return -1;

		*pq31_dst++ = MUL__Q31(q31_in_left,  p_delay->q31_dry) + MUL__Q31(q31_out_left,  p_delay->q31_wet);
		*pq31_dst++ = MUL__Q31(q31_in_right, p_delay->q31_dry) + MUL__Q31(q31_out_right, p_delay->q31_wet);
	} while (--u32_buffer_size);

	p_delay->q31_out_left  = q31_out_left;
	p_delay->q31_out_right = q31_out_right;

	return 0;
}
