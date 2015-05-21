#ifndef DELAY_LINE_H
#define DELAY_LINE_H

#include <fixedpoint.h>

/* Init a delay line intance */
void * delay_line_init(uint32_t u32_size);

/* Deinit a delay line intance */
int delay_line_deinit(void *p_handle);

/* Flush delay buffer */
int delay_line_flush(void *p_handle);

/* Set delay in samples */
int delay_line_set_delay(void *p_handle, uint32_t u32_delay);

/* Put sample into delay line */
int delay_line_do(void *p_handle, q31_t *pq31_in, q31_t *pq31_out);

#endif
