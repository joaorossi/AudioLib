#ifndef FRAC_DELAY_LINE_H
#define FRAC_DELAY_LINE_H

#include <fixedpoint.h>

/* Init a fractional delay line intance */
void * frac_delay_line_init(uint32_t u32_size);

/* Deinit a fractional delay line intance */
int frac_delay_line_deinit(void *p_handle);

/* Flush delay buffer */
int frac_delay_line_flush(void *p_handle);

/* Set delay in samples */
int frac_delay_line_set_delay(void *p_handle, double f64_delay);

/* Put sample into delay line */
int frac_delay_line_do(void *p_handle, q31_t *pq31_in, q31_t *pq31_out);

#endif
