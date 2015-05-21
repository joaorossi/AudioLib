#ifndef WAVE_TABLE_OSC_H_
#define WAVE_TABLE_OSC_H_

#include <fixedpoint.h>

/* Init a WTO instance */
void * wave_table_osc_init(uint32_t u32_size);

/* Deinit a WTO instance */
int wave_table_osc_deinit(void *p_handle);

/* Flush opsc state */
int wave_table_osc_flush(void *p_handle);

/* Set wave type */
int wave_table_osc_set_type(void *p_handle, uint32_t u32_type);

/* Set table increment */
int wave_table_osc_set_incr(void *p_handle, double f64_incr);

/* Set table index offset */
int wave_table_osc_set_offset(void *p_handle, double f64_offset);

/* Realize wave table */
int wave_table_osc_do(void *p_handle, q31_t *pq31_dst, uint32_t u32_buffer_size);

#endif
