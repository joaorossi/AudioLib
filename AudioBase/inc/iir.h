#ifndef IIR_H
#define IIR_H

#include <fixedpoint.h>

/* Init a iir filter instance */
void * iir_init();

/* Deinit a iir filter instance */
int iir_deinit(void *p_handle);

/* Flush filter states and coeffs */
int iir_flush(void *p_handle);

/* Set vector of coefficients */
int iir_set_coeffs(void *p_handle, q31_t *pq31_coeffs, uint32_t u32_n);

/* Set number of biquad stages */
int iir_set_stages(void *p_handle, uint32_t u32_n);

/* Set multiply accumulate shift */
int iir_set_shift(void *p_handle, uint32_t u32_n);

/* Realize biquad iir structure */
int iir_do(void *p_handle, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size);

#endif
