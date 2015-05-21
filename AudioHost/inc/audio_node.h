#ifndef AUDIO_NODE_H
#define AUDIO_NODE_H

#include <fixedpoint.h>

/************************************************
 *					Definitions					*
 ************************************************/
enum {
	MONO_TO_MONO = 0,
	STEREO_TO_STEREO,
	MONO_TO_STEREO
};

/************************************************
 *					AudioNode					*
 ************************************************/
typedef struct AudioNode {

	/* Plugin slot of node */
	void      *p_plugin_slot;

	/* Links to adjunct nodes */
	void     *p_next, *p_prev;

	/* Audio buffer */
	q31_t    *pq31_buffer;
	uint32_t u32_buffer_size;

	/* Channel mode */
	uint32_t u32_mode;

	/* Belong to some chain */
	uint32_t u32_in_chain;

	/* Sampling rate */
	double   f64_sampling_rate;

} AudioNode_t;

/************************************************
 *					AudioNode API				*
 ************************************************/
AudioNode_t * new_audio_node(double f64_sampling_rate, uint32_t u32_mode, uint32_t u32_buffer_size);

int free_audio_node(AudioNode_t *p_audio_node);

int plugin_load(AudioNode_t *p_audio_node, char *str_plugin_path);

int plugin_unload(AudioNode_t *p_audio_node);

int plugin_get_param(AudioNode_t *p_audio_node, uint32_t u32_param, void *p_value);

int plugin_set_param(AudioNode_t *p_audio_node, uint32_t u32_param, void *p_value);

int plugin_process(AudioNode_t *p_audio_node, q31_t *pq31_in_buffer);

#endif