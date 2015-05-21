#include <fixedpoint.h>
#include <dlfcn.h>

/****************************************************************
 *						Plugin definitions						*
 ****************************************************************/

/* Filter plugin path */
#define FILTER_PATH "/Users/joaorossifilho/Documents/Cronus/Labs/Z-Core/Plugins/bin/libFilter.dylib"

/* Filter plugin param id's */
#define FILTER_TYPE 14
#define BYPS_PARAM 0
#define FREQ_PARAM 1
#define RESO_PARAM 2
#define GAIN_PARAM 3
#define TYPE_PARAM 4

/* Band freqs */
#define FREQ_1  32.0
#define FREQ_2  64.0
#define FREQ_3  125.0
#define FREQ_4  250.0
#define FREQ_5  500.0
#define FREQ_6  1000.0
#define FREQ_7  2000.0
#define FREQ_8  4000.0
#define FREQ_9  8000.0
#define FREQ_10 16000.0

/* Bands filter reso */
#define RESO 1.414212562

/* Parameters enum */
enum {
	BYPASS = 0,

	BAND_1, BAND_2, BAND_3, BAND_4, BAND_5,
	BAND_6, BAND_7, BAND_8, BAND_9, BAND_10
};

/* Plugin struct */
struct GraphicEQ {
	/* Filter instances */
	void     *p_filter_1, *p_filter_2, *p_filter_3, *p_filter_4, *p_filter_5,
		     *p_filter_6, *p_filter_7, *p_filter_8, *p_filter_9, *p_filter_10;

	/* Eq gain values */
	double   f64_gain_1, f64_gain_2, f64_gain_3, f64_gain_4, f64_gain_5,
			 f64_gain_6, f64_gain_7, f64_gain_8, f64_gain_9, f64_gain_10;

	/* Bypass */
	uint32_t u32_bypass;

	/* Sampling frequency */
	double	 f64_sampling_freq;
};


/****************************************************************
 *					Dynamic Libraries Pointers					*
 ****************************************************************/

static void *p_filter_handle;

static void * (*filter_init)                          (double);
static int    (*filter_deinit)                        (void *);
static int    (*filter_set_param)                     (void *, int, void *);
static int    (*filter_process__MONO_IN_MONO_OUT)     (void *, q31_t *, q31_t *, uint32_t);
static int    (*filter_process__MONO_IN_STEREO_OUT)   (void *, q31_t *, q31_t *, uint32_t);
static int    (*filter_process__STEREO_IN_STEREO_OUT) (void *, q31_t *, q31_t *, uint32_t);

/****************************************************************
 *						Public functions						*
 ****************************************************************/

void *
plugin_init(double f64_sampling_freq)
{
	struct   GraphicEQ *p_eq = calloc(1, sizeof(struct GraphicEQ));
	double   f64_reso = RESO, f64_freq;
	uint32_t u32_filt_type = FILTER_TYPE;

	if (!p_eq)
		  return NULL;

	if (f64_sampling_freq < 0.0)
		goto cleanup;

	p_eq->f64_sampling_freq = f64_sampling_freq;

	/* Load Filter plugin */
	p_filter_handle = dlopen(FILTER_PATH, RTLD_NOW);

	if (dlerror())
		goto cleanup;

	filter_init                          = dlsym(p_filter_handle, "plugin_init");
	filter_deinit                        = dlsym(p_filter_handle, "plugin_deinit");
	filter_set_param                     = dlsym(p_filter_handle, "plugin_set_param");
	filter_process__MONO_IN_MONO_OUT     = dlsym(p_filter_handle, "plugin_process__MONO_IN_MONO_OUT");
	filter_process__MONO_IN_STEREO_OUT   = dlsym(p_filter_handle, "plugin_process__MONO_IN_STEREO_OUT");
	filter_process__STEREO_IN_STEREO_OUT = dlsym(p_filter_handle, "plugin_process__STEREO_IN_STEREO_OUT");

	if (dlerror()) 
		goto cleanup;

	if (!(p_eq->p_filter_1  = filter_init(p_eq->f64_sampling_freq)) ||
		!(p_eq->p_filter_2  = filter_init(p_eq->f64_sampling_freq)) ||
		!(p_eq->p_filter_3  = filter_init(p_eq->f64_sampling_freq)) ||
		!(p_eq->p_filter_4  = filter_init(p_eq->f64_sampling_freq)) ||
		!(p_eq->p_filter_5  = filter_init(p_eq->f64_sampling_freq)) ||
		!(p_eq->p_filter_6  = filter_init(p_eq->f64_sampling_freq)) ||
		!(p_eq->p_filter_7  = filter_init(p_eq->f64_sampling_freq)) ||
		!(p_eq->p_filter_8  = filter_init(p_eq->f64_sampling_freq)) ||
		!(p_eq->p_filter_9  = filter_init(p_eq->f64_sampling_freq)) ||
		!(p_eq->p_filter_10 = filter_init(p_eq->f64_sampling_freq)))
		goto cleanup;

	f64_freq = FREQ_1;
	p_eq->f64_gain_1 = 0.0;
	if (filter_set_param(p_eq->p_filter_1, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_1, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_1, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_1, GAIN_PARAM, &(p_eq->f64_gain_1)))
		goto cleanup;

	f64_freq = FREQ_2;
	p_eq->f64_gain_2 = 0.0;
	if (filter_set_param(p_eq->p_filter_2, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_2, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_2, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_2, GAIN_PARAM, &(p_eq->f64_gain_2)))
		goto cleanup;

	f64_freq = FREQ_3;
	p_eq->f64_gain_3 = 0.0;
	if (filter_set_param(p_eq->p_filter_3, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_3, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_3, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_3, GAIN_PARAM, &(p_eq->f64_gain_3)))
		goto cleanup;

	f64_freq = FREQ_4;
	p_eq->f64_gain_4 = 0.0;
	if (filter_set_param(p_eq->p_filter_4, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_4, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_4, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_4, GAIN_PARAM, &(p_eq->f64_gain_4)))
		goto cleanup;

	f64_freq = FREQ_5;
	p_eq->f64_gain_5 = 0.0;
	if (filter_set_param(p_eq->p_filter_5, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_5, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_5, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_5, GAIN_PARAM, &(p_eq->f64_gain_5)))
		goto cleanup;

	f64_freq = FREQ_6;
	p_eq->f64_gain_6 = 0.0;
	if (filter_set_param(p_eq->p_filter_6, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_6, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_6, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_6, GAIN_PARAM, &(p_eq->f64_gain_6)))
		goto cleanup;

	f64_freq = FREQ_7;
	p_eq->f64_gain_7 = 0.0;
	if (filter_set_param(p_eq->p_filter_7, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_7, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_7, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_7, GAIN_PARAM, &(p_eq->f64_gain_7)))
		goto cleanup;

	f64_freq = FREQ_8;
	p_eq->f64_gain_8 = 0.0;
	if (filter_set_param(p_eq->p_filter_8, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_8, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_8, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_8, GAIN_PARAM, &(p_eq->f64_gain_8)))
		goto cleanup;

	f64_freq = FREQ_9;
	p_eq->f64_gain_9 = 0.0;
	if (filter_set_param(p_eq->p_filter_9, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_9, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_9, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_9, GAIN_PARAM, &(p_eq->f64_gain_9)))
		goto cleanup;

	f64_freq = FREQ_10;
	p_eq->f64_gain_10 = 0.0;
	if (filter_set_param(p_eq->p_filter_10, TYPE_PARAM, &u32_filt_type) ||
		filter_set_param(p_eq->p_filter_10, FREQ_PARAM, &f64_freq)      ||
		filter_set_param(p_eq->p_filter_10, RESO_PARAM, &f64_reso)      ||
		filter_set_param(p_eq->p_filter_10, GAIN_PARAM, &(p_eq->f64_gain_10)))
		goto cleanup;

	return p_eq;

cleanup:

	if (p_filter_handle) {
		if (filter_deinit) {
			filter_deinit(p_eq->p_filter_1);
			filter_deinit(p_eq->p_filter_2);
			filter_deinit(p_eq->p_filter_3);
			filter_deinit(p_eq->p_filter_4);
			filter_deinit(p_eq->p_filter_5);
			filter_deinit(p_eq->p_filter_6);
			filter_deinit(p_eq->p_filter_7);
			filter_deinit(p_eq->p_filter_8);
			filter_deinit(p_eq->p_filter_9);
			filter_deinit(p_eq->p_filter_10);
		}

		dlclose(p_filter_handle);

		p_filter_handle                      = NULL;
		filter_init                          = NULL;
		filter_deinit                        = NULL;
		filter_set_param                     = NULL;
		filter_process__MONO_IN_MONO_OUT     = NULL;
		filter_process__MONO_IN_STEREO_OUT   = NULL;
		filter_process__STEREO_IN_STEREO_OUT = NULL;
	}

	free(p_eq);

	return NULL;
}

int
plugin_deinit(void *p_plugin)
{
	struct GraphicEQ *p_eq = p_plugin;

	if (!p_eq || !p_filter_handle)
		return -1;

	filter_deinit(p_eq->p_filter_1);
	filter_deinit(p_eq->p_filter_2);
	filter_deinit(p_eq->p_filter_3);
	filter_deinit(p_eq->p_filter_4);
	filter_deinit(p_eq->p_filter_5);
	filter_deinit(p_eq->p_filter_6);
	filter_deinit(p_eq->p_filter_7);
	filter_deinit(p_eq->p_filter_8);
	filter_deinit(p_eq->p_filter_9);
	filter_deinit(p_eq->p_filter_10);

	free(p_eq);

	dlclose(p_filter_handle);

	return 0;
}

int
plugin_set_param(void *p_plugin, int param, void *p_value)
{
	struct GraphicEQ *p_eq = p_plugin;

	if (!p_eq || !p_value || !p_filter_handle)
		return -1;

	switch (param) {
		case BYPASS:
			if ((*((uint32_t *)(p_value)) != 1) && (*((uint32_t *)(p_value)) != 0))
				return -1;

			memcpy(&(p_eq->u32_bypass), p_value, sizeof(uint32_t));
			break;

		case BAND_1:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_1), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_1, GAIN_PARAM, &(p_eq->f64_gain_1)))
				return -1;
			break;

		case BAND_2:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_2), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_2, GAIN_PARAM, &(p_eq->f64_gain_2)))
				return -1;
			break;

		case BAND_3:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_3), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_3, GAIN_PARAM, &(p_eq->f64_gain_3)))
				return -1;
			break;

		case BAND_4:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_4), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_4, GAIN_PARAM, &(p_eq->f64_gain_4)))
				return -1;
			break;

		case BAND_5:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_5), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_5, GAIN_PARAM, &(p_eq->f64_gain_5)))
				return -1;
			break;

		case BAND_6:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_6), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_6, GAIN_PARAM, &(p_eq->f64_gain_6)))
				return -1;
			break;

		case BAND_7:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_7), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_7, GAIN_PARAM, &(p_eq->f64_gain_7)))
				return -1;
			break;

		case BAND_8:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_8), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_8, GAIN_PARAM, &(p_eq->f64_gain_8)))
				return -1;
			break;

		case BAND_9:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_9), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_9, GAIN_PARAM, &(p_eq->f64_gain_9)))
				return -1;
			break;

		case BAND_10:
			if ((*((double *)(p_value)) > 48.0) || (*((double *)(p_value)) < -48.0))
				return -1;

			memcpy(&(p_eq->f64_gain_10), p_value, sizeof(double));

			if (filter_set_param(p_eq->p_filter_10, GAIN_PARAM, &(p_eq->f64_gain_10)))
				return -1;
			break;

		default:
			return -1;
	}

	return 0;
}

int
plugin_get_param(void *p_plugin, int param, void *p_value)
{
	struct GraphicEQ *p_eq = p_plugin;

	if (!p_eq || !p_value || !p_filter_handle)
		return -1;

	switch (param) {
		case BYPASS:
			memcpy(p_value, &(p_eq->u32_bypass), sizeof(uint32_t));
			break;

		case BAND_1:
			memcpy(p_value, &(p_eq->f64_gain_1), sizeof(double));
			break;

		case BAND_2:
			memcpy(p_value, &(p_eq->f64_gain_2), sizeof(double));
			break;

		case BAND_3:
			memcpy(p_value, &(p_eq->f64_gain_3), sizeof(double));
			break;

		case BAND_4:
			memcpy(p_value, &(p_eq->f64_gain_4), sizeof(double));
			break;

		case BAND_5:
			memcpy(p_value, &(p_eq->f64_gain_5), sizeof(double));
			break;

		case BAND_6:
			memcpy(p_value, &(p_eq->f64_gain_6), sizeof(double));
			break;

		case BAND_7:
			memcpy(p_value, &(p_eq->f64_gain_7), sizeof(double));
			break;

		case BAND_8:
			memcpy(p_value, &(p_eq->f64_gain_8), sizeof(double));
			break;

		case BAND_9:
			memcpy(p_value, &(p_eq->f64_gain_9), sizeof(double));
			break;

		case BAND_10:
			memcpy(p_value, &(p_eq->f64_gain_10), sizeof(double));
			break;

		default:
			return -1;
	}

	return 0;
}

int
plugin_process__MONO_IN_MONO_OUT(void *p_plugin,
								 q31_t *pq31_src, q31_t *pq31_dst,
								 uint32_t u32_buffer_size)
{
	struct GraphicEQ *p_eq = p_plugin;

	if (!p_eq || !pq31_src || !pq31_dst || !p_filter_handle)
		return -1;

	if (p_eq->u32_bypass) {
		memcpy(pq31_dst, pq31_src, u32_buffer_size * sizeof(q31_t));
		return 0;
	}

	q31_t pq31_aux[u32_buffer_size];

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_1, pq31_src, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_2, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_3, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_4, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_5, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_6, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_7, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_8, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_9, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__MONO_IN_MONO_OUT(p_eq->p_filter_10, pq31_aux, pq31_dst, u32_buffer_size))
		return -1;

	return 0;
}

int
plugin_process__MONO_IN_STEREO_OUT(void *p_plugin,
								   q31_t *pq31_src, q31_t *pq31_dst,
								   uint32_t u32_buffer_size)
{
	struct GraphicEQ *p_eq = p_plugin;

	if (!p_eq || !pq31_src || !pq31_dst || !p_filter_handle)
		return -1;

	if (p_eq->u32_bypass) {
		do {
			*pq31_dst++ = *pq31_src;
			*pq31_dst++ = *pq31_src++;
		} while (--u32_buffer_size);
		return 0;
	}

	q31_t pq31_aux[2*u32_buffer_size];

	if (filter_process__MONO_IN_STEREO_OUT(p_eq->p_filter_1, pq31_src, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_2, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_3, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_4, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_5, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_6, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_7, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_8, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_9, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_10, pq31_aux, pq31_dst, u32_buffer_size))
		return -1;

	return 0;
}

int
plugin_process__STEREO_IN_STEREO_OUT(void *p_plugin,
									 q31_t *pq31_src, q31_t *pq31_dst,
									 uint32_t u32_buffer_size)
{
	struct GraphicEQ *p_eq = p_plugin;

	if (!p_eq || !pq31_src || !pq31_dst || !p_filter_handle)
		return -1;

	if (p_eq->u32_bypass) {
		memcpy(pq31_dst, pq31_src, 2 * u32_buffer_size * sizeof(q31_t));
		return 0;
	}

	q31_t pq31_aux[2*u32_buffer_size];

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_1, pq31_src, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_2, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_3, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_4, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_5, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_6, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_7, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_8, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_9, pq31_aux, pq31_aux, u32_buffer_size))
		return -1;

	if (filter_process__STEREO_IN_STEREO_OUT(p_eq->p_filter_10, pq31_aux, pq31_dst, u32_buffer_size))
		return -1;

	return 0;
}
