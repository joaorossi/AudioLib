#include <fixedpoint.h>
#include <frac_delay_line.h>
#include <wave_table_osc.h>

/****************************************************************
 *						Plugin definitions						*
 ****************************************************************/

#define FDL_BUFFER_SIZE 48000
#define WTO_TABLE_SIZE  16384

enum {
	SIN = 0,
	TRI,
	SAW,
	SQR
};

enum {
	BYPASS = 0,
	RATE,
	FEEDBACK,
	DEPTH,
	WAVE,
	STEREO
};

struct Flanger {
	void *p_fdl_left, *p_fdl_right;

	void *p_wto_left, *p_wto_right;

	double f64_rate_hz, f64_feedback_pct, f64_depth_sec;

	q31_t q31_feedback;

	q31_t q31_out_left, q31_out_right;

	uint32_t u32_wave;

	uint32_t u32_stereo;

	uint32_t u32_bypass;

	double f64_sampling_freq;
};

/****************************************************************
 *						Public functions						*
 ****************************************************************/

void *
plugin_init(double f64_sampling_freq)
{
	struct Flanger *p_flanger = calloc(1, sizeof(struct Flanger));

	if (!p_flanger)
		return NULL;

	if (f64_sampling_freq <= 0.0)
		goto cleanup;

	/* Initialize fractional delay lines and wave table oscillators instances */
	if (!(p_flanger->p_fdl_left  = frac_delay_line_init(FDL_BUFFER_SIZE)) ||
		!(p_flanger->p_fdl_right = frac_delay_line_init(FDL_BUFFER_SIZE)) ||
		!(p_flanger->p_wto_left  = wave_table_osc_init(WTO_TABLE_SIZE))   ||
		!(p_flanger->p_wto_right = wave_table_osc_init(WTO_TABLE_SIZE)))
		goto cleanup;

	p_flanger->f64_sampling_freq = f64_sampling_freq;

	return p_flanger;

cleanup:

	wave_table_osc_deinit(p_flanger->p_wto_left);
	wave_table_osc_deinit(p_flanger->p_wto_right);

	frac_delay_line_deinit(p_flanger->p_fdl_left);
	frac_delay_line_deinit(p_flanger->p_fdl_right);

	free(p_flanger);

	return NULL;
}

int
plugin_deinit(void *p_plugin)
{
	struct Flanger *p_flanger = p_plugin;

	if (!p_flanger)
		return -1;

	wave_table_osc_deinit(p_flanger->p_wto_left);
	wave_table_osc_deinit(p_flanger->p_wto_right);

	frac_delay_line_deinit(p_flanger->p_fdl_left);
	frac_delay_line_deinit(p_flanger->p_fdl_right);

	free(p_flanger);

	return 0;
}

int
plugin_set_param(void *p_plugin, int param, void *p_value)
{
	struct Flanger *p_flanger = p_plugin;

	double   f64_rate_hz, f64_feedback_pct, f64_feedback, f64_depth_sec, f64_incr;
	uint32_t u32_bypass, u32_stereo, u32_wave;

	if (!p_flanger || !p_value)
		return -1;

	switch (param) {
		case BYPASS:
			u32_bypass = *((uint32_t *)(p_value));

			if ((u32_bypass != 1) && (u32_bypass != 0))
				return -1;

			if (p_flanger->u32_bypass == u32_bypass)
				return 0;

			if (u32_bypass) {
				if (frac_delay_line_flush(p_flanger->p_fdl_left)  ||
					frac_delay_line_flush(p_flanger->p_fdl_right) ||
					wave_table_osc_flush(p_flanger->p_wto_left)   ||
					wave_table_osc_flush(p_flanger->p_wto_right))
					return -1;
			}

			p_flanger->u32_bypass = u32_bypass;
			break;

		case RATE:
			f64_rate_hz = *((double *)(p_value));

			if ((f64_rate_hz <= 0.0) || (f64_rate_hz > 10.0))
				return -1;

			p_flanger->f64_rate_hz = f64_rate_hz;
			f64_incr = (p_flanger->f64_rate_hz * WTO_TABLE_SIZE)/p_flanger->f64_sampling_freq;

			if (wave_table_osc_set_incr(p_flanger->p_wto_left, f64_incr) ||
				wave_table_osc_set_incr(p_flanger->p_wto_right, f64_incr))
				return -1;
			break;

		case FEEDBACK:
			f64_feedback_pct = *((double *)(p_value));

			if ((f64_feedback_pct > 100.0) || (f64_feedback_pct < 0.0))
				return -1;

			p_flanger->f64_feedback_pct = f64_feedback_pct;
			f64_feedback = p_flanger->f64_feedback_pct/100.0;
			p_flanger->q31_feedback = F64_TO_Q31(f64_feedback);
			break;

		case DEPTH:
			f64_depth_sec = *((double *)(p_value));

			if ((f64_depth_sec > 100.0) || (f64_depth_sec < 0.0))
				return -1;

			p_flanger->f64_depth_sec = f64_depth_sec;
			break;

		case WAVE:
			u32_wave = *((uint32_t *)(p_value));

			if (u32_wave > SQR)
				return -1;

			p_flanger->u32_wave = u32_wave;

			if (wave_table_osc_flush(p_flanger->p_wto_left)  ||
				wave_table_osc_flush(p_flanger->p_wto_right) ||
				frac_delay_line_flush(p_flanger->p_fdl_left) ||
				frac_delay_line_flush(p_flanger->p_fdl_right))
				return -1;

			if (wave_table_osc_set_type(p_flanger->p_wto_left, p_flanger->u32_wave) ||
				wave_table_osc_set_type(p_flanger->p_wto_right, p_flanger->u32_wave))
				return -1;
			break;

		case STEREO:
			u32_stereo = *((uint32_t *)(p_value));

			if ((u32_stereo != 1) && (u32_stereo != 0))
				return -1;

			if (p_flanger->u32_stereo == u32_stereo)
				return 0;

			p_flanger->u32_stereo = u32_stereo;

			if (p_flanger->u32_stereo) {
				if (wave_table_osc_set_offset(p_flanger->p_wto_right, WTO_TABLE_SIZE/4))
					return -1;
			} else {
				if (wave_table_osc_set_offset(p_flanger->p_wto_right, 0))
					return -1;
			}
			break;

		default:
			return -1;
	}

	return 0;
}

int
plugin_get_param(void *p_plugin, int param, void *p_value)
{
	struct Flanger *p_flanger = p_plugin;

	if (!p_flanger || !p_value)
		return -1;

	switch (param) {
		case BYPASS:
			memcpy(p_value, &(p_flanger->u32_bypass), sizeof(uint32_t));
			break;

		case RATE:
			memcpy(p_value, &(p_flanger->f64_rate_hz), sizeof(double));
			break;

		case FEEDBACK:
			memcpy(p_value, &(p_flanger->f64_feedback_pct), sizeof(double));
			break;

		case DEPTH:
			memcpy(p_value, &(p_flanger->f64_depth_sec), sizeof(double));
			break;

		case WAVE:
			memcpy(p_value, &(p_flanger->u32_wave), sizeof(uint32_t));
			break;

		case STEREO:
			memcpy(p_value, &(p_flanger->u32_stereo), sizeof(uint32_t));
			break;

		default:
			return -1;
	}

	return 0;
}

int
plugin_process__MONO_IN_MONO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct Flanger *p_flanger = p_plugin;

	q31_t   q31_in, q31_out, q31_fdb;
	double  f64_wto_delay, f64_depth_smp;
	int32_t i32_i = 0;

	q31_t pq31_wto[u32_buffer_size];

	if (!p_flanger || !pq31_src || !pq31_dst)
		return -1;

	if (p_flanger->u32_bypass) {
		memcpy(pq31_dst, pq31_src, u32_buffer_size * sizeof(q31_t));
		return 0;
	}

	if (wave_table_osc_do(p_flanger->p_wto_left, pq31_wto, u32_buffer_size))
		return -1;

	q31_out       = p_flanger->q31_out_left;
	f64_depth_smp = p_flanger->f64_depth_sec * p_flanger->f64_sampling_freq * 0.5;

	do {
		q31_in = *pq31_src++;

		q31_fdb = MUL__Q31(q31_out, p_flanger->q31_feedback);
		q31_fdb >>= 1;
		q31_fdb += q31_in >> 1;

		if (frac_delay_line_do(p_flanger->p_fdl_left, &q31_fdb, &q31_out))
			return -1;

		*pq31_dst++ = (q31_in >> 1) + (q31_out >> 1);

		f64_wto_delay = f64_depth_smp * (1.0 + Q31_TO_F64(pq31_wto[i32_i++]));

		if (frac_delay_line_set_delay(p_flanger->p_fdl_left, f64_wto_delay))
			return -1;
	} while (--u32_buffer_size);

	p_flanger->q31_out_left = q31_out;

	return 0;
}

int
plugin_process__MONO_IN_STEREO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct Flanger *p_flanger = p_plugin;

	q31_t   q31_in_left, q31_in_right, q31_fdb_left, q31_fdb_right, q31_out_left, q31_out_right;
	double  f64_wto_delay_left, f64_wto_delay_right, f64_depth_smp;
	int32_t i32_i = 0;

	q31_t pq31_wto_left[u32_buffer_size], pq31_wto_right[u32_buffer_size];

	if (!p_flanger || !pq31_src || !pq31_dst)
		return -1;

	if (p_flanger->u32_bypass) {
		do {
			*pq31_dst++ = *pq31_src;
			*pq31_dst++ = *pq31_src++;
		} while (--u32_buffer_size);
		return 0;
	}

	if (wave_table_osc_do(p_flanger->p_wto_left, pq31_wto_left, u32_buffer_size) ||
		wave_table_osc_do(p_flanger->p_wto_right, pq31_wto_right, u32_buffer_size))
		return -1;

	q31_out_left  = p_flanger->q31_out_left;
	q31_out_right = p_flanger->q31_out_right;

	f64_depth_smp = p_flanger->f64_depth_sec * p_flanger->f64_sampling_freq * 0.5;

	do {
		q31_in_left  = *pq31_src;
		q31_in_right = *pq31_src++;

		q31_fdb_left  = MUL__Q31(q31_out_left, p_flanger->q31_feedback);
		q31_fdb_right = MUL__Q31(q31_out_right, p_flanger->q31_feedback);

		q31_fdb_left >>= 1;
		q31_fdb_right >>= 1;

		q31_fdb_left  += q31_in_left >> 1;
		q31_fdb_right += q31_in_right >> 1;

		if (frac_delay_line_do(p_flanger->p_fdl_left, &q31_fdb_left, &q31_out_left) ||
			frac_delay_line_do(p_flanger->p_fdl_right, &q31_fdb_right, &q31_out_right))
			return -1;

		*pq31_dst++ = q31_out_left;
		*pq31_dst++ = q31_out_right;

		f64_wto_delay_left  = f64_depth_smp * (1.0 + Q31_TO_F64(pq31_wto_left[i32_i]));
		f64_wto_delay_right = f64_depth_smp * (1.0 + Q31_TO_F64(pq31_wto_right[i32_i]));
		i32_i++;

		if (frac_delay_line_set_delay(p_flanger->p_fdl_left,  f64_wto_delay_left) ||
			frac_delay_line_set_delay(p_flanger->p_fdl_right, f64_wto_delay_right))
			return -1;
	} while (--u32_buffer_size);

	p_flanger->q31_out_left  = q31_out_left;
	p_flanger->q31_out_right = q31_out_right;

	return 0;
}

int
plugin_process__STEREO_IN_STEREO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	struct Flanger *p_flanger = p_plugin;

	q31_t   q31_in_left, q31_in_right, q31_fdb_left, q31_fdb_right, q31_out_left, q31_out_right;
	double  f64_wto_delay_left, f64_wto_delay_right, f64_depth_smp;
	int32_t i32_i = 0;

	q31_t pq31_wto_left[u32_buffer_size], pq31_wto_right[u32_buffer_size];

	if (!p_flanger || !pq31_src || !pq31_dst)
		return -1;

	if (p_flanger->u32_bypass) {
		memcpy(pq31_dst, pq31_src, 2 * u32_buffer_size * sizeof(q31_t));
		return 0;
	}

	if (wave_table_osc_do(p_flanger->p_wto_left, pq31_wto_left, u32_buffer_size) ||
		wave_table_osc_do(p_flanger->p_wto_right, pq31_wto_right, u32_buffer_size))
		return -1;

	q31_out_left  = p_flanger->q31_out_left;
	q31_out_right = p_flanger->q31_out_right;

	f64_depth_smp = p_flanger->f64_depth_sec * p_flanger->f64_sampling_freq * 0.5;

	do {
		q31_in_left  = *pq31_src++;
		q31_in_right = *pq31_src++;

		q31_fdb_left  = MUL__Q31(q31_out_left, p_flanger->q31_feedback);
		q31_fdb_right = MUL__Q31(q31_out_right, p_flanger->q31_feedback);

		q31_fdb_left >>= 1;
		q31_fdb_right >>= 1;

		q31_fdb_left  += q31_in_left >> 1;
		q31_fdb_right += q31_in_right >> 1;

		if (frac_delay_line_do(p_flanger->p_fdl_left, &q31_fdb_left, &q31_out_left) ||
			frac_delay_line_do(p_flanger->p_fdl_right, &q31_fdb_right, &q31_out_right))
			return -1;

		*pq31_dst++ = q31_out_left;
		*pq31_dst++ = q31_out_right;

		f64_wto_delay_left  = f64_depth_smp * (1.0 + Q31_TO_F64(pq31_wto_left[i32_i]));
		f64_wto_delay_right = f64_depth_smp * (1.0 + Q31_TO_F64(pq31_wto_right[i32_i]));
		i32_i++;

		if (frac_delay_line_set_delay(p_flanger->p_fdl_left,  f64_wto_delay_left) ||
			frac_delay_line_set_delay(p_flanger->p_fdl_right, f64_wto_delay_right))
			return -1;
	} while (--u32_buffer_size);

	p_flanger->q31_out_left  = q31_out_left;
	p_flanger->q31_out_right = q31_out_right;

	return 0;
}
