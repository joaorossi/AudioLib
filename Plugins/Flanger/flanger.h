#ifndef FLANGER_H
#define FLANGER_H

#include <fixedpoint.h>

/* Plugin instance initializer */
void * plugin_init(double f64_sampling_freq);

/* Plugin instance deinitializer */
int plugin_deinit(void *p_plugin);

/* Setter */
int plugin_set_param(void *p_plugin, int param, void *p_value);

/* Getter */
int plugin_get_param(void *p_plugin, int param, void *p_value);

/* Audio processing callbacks */
int plugin_process__MONO_IN_MONO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size);

int plugin_process__MONO_IN_STEREO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size);

int plugin_process__STEREO_IN_STEREO_OUT(void *p_plugin, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size);

#endif
