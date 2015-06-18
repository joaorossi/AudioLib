#ifndef OVERSAMPLER_H
#define OVERSAMPLER_H

#include <fixedpoint.h>

/**
 * Initialize oversampler structure
 *
 * @param	f64_sample_rate:		Original sample rate of signal.
 *
 * @param	u32_orig_buffer_len:	Length in samples of original sample rate signal.
 *
 * @param	u32_ovrsmpl_factor:		Oversampling factor
 *
 * @return	void *:					Reference to oversampler instance
 */
void * oversampler_init(f64_t f64_orig_sample_rate, uint32_t u32_orig_buffer_len, uint32_t u32_ovrsmp_factor);


/** 
 * Deinitialize oversampler structure 
 *
 * @param	*p_ovrsmp:	Reference to ovesampler instance. The reference
 * 						becomes invalid after a call to this function.
 *
 * @return	int:		0 on success, -1 on error
 */
int	oversampler_deinit(void *p_ovrsmp);


/** 
 * Flush internal states
 *
 * @param	*p_ovrsmpl:	Reference to ovesampler instance
 *
 * @return	int:		0 on success, -1 on error
 */
int	oversampler_flush(void *p_ovrsmp);

/**
 * Get oversampled buffers used by the lib. The buffers are allocated
 * by the lib and automatically freed when oversampler_deinit function
 * is called. Do not pass those pointers around the application, they are
 * intended for use within the oversampling context only. Both buffers
 * are u32_orig_buffer_len*u32_ovrsmp_factor samples long.
 *
 * @param	**pq31_ovrsmp_out:	Buffer filled by oversampler_interpolate function.
 *
 * @param	**pq31_ovrsmp_in:	Buffer used by oversampler_decimate function.
 *
 * @return	int:				0 on sucess, -1 on error
 */
int oversampler_get_oversampled_buffers(void *p_ovrsmp, q31_t **pq31_ovrsmp_out, q31_t **pq31_ovrsmp_in);

/**
 * Upsample and interpolate. After a call to this function the pq31_ovrsmp_out buffer will be
 * filled with the upsampled signal.
 *
 * @param	*p_ovrsmp:			Reference to oversampler instance.
 *
 * @param	*pq31_orig_in:		Input buffer at original sample rate. This buffer
 * 								should be provided by calling code and be u32_orig_buffer_len
 * 								samples long.
 *
 * @return	int:				0 on success, -1 on error
 */
int	oversampler_interpolate(void *p_ovrsmp, q31_t *pq31_orig_in);


/**
 * Downsample and decimate. Sampled stored at pq31_ovrsmp_in will be used by
 * the decimation engine.
 *
 * @param	*p_ovrsmp:			Reference to oversampler instance.
 *
 * @param	*pq31_orig_out:		Output buffer at original sample rate. This buffer
 * 								should be provided by calling code and be u32_orig_buffer_len
 * 								samples long.
 *
 * @return	int:				0 on success, -1 on error
 */
int oversampler_decimate(void *p_ovrsmp, q31_t *pq31_orig_out);

#endif

