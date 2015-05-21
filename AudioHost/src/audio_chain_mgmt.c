#include <audio_chain_mgmt.h>

AudioChain_t *
load_audio_chain(uint32_t u32_node_count, uint32_t u32_mode,
				 uint32_t u32_buffer_size, double f64_sampling_rate)
{
	AudioChain_t *p_audio_chain = NULL;
	AudioNode_t  **p_audio_node_vec = NULL;

	uint32_t i;

	if ((u32_mode > MONO_TO_STEREO)         ||
		(u32_node_count > MAX_NODE_COUNT)   ||
		(u32_buffer_size > MAX_BUFFER_SIZE) ||
		(f64_sampling_rate > MAX_SAMPLING_RATE))
		return NULL;

	if (!(p_audio_chain = new_audio_chain()))
		goto error;

	if (!(p_audio_node_vec = calloc(1, u32_node_count * sizeof(AudioNode_t *))))
		goto error;

	for (i = 0; i < u32_node_count; i++) {
		if (!(p_audio_node_vec[i] = new_audio_node(f64_sampling_rate, u32_mode, u32_buffer_size)))
			goto error;

		if (append_node_to_chain(p_audio_chain, p_audio_node_vec[i]))
			goto error;
	}

	if (p_audio_chain->u32_count != u32_node_count)
		goto error;

	if (p_audio_node_vec)
		free(p_audio_node_vec);

	return p_audio_chain;

error:

	if (p_audio_chain)
		free_audio_chain(p_audio_chain);

	if (p_audio_node_vec) {
		for (i = 0; i <  u32_node_count; i++) {
			if (free_audio_node(p_audio_node_vec[i]))
				break;
		}

		free(p_audio_node_vec);
	}

	return NULL;
}

int
unload_audio_chain(AudioChain_t *p_audio_chain)
{
	AudioNode_t **p_audio_node_vec;

	uint32_t u32_count, i;

	if (!p_audio_chain)
		return -1;

	if ((u32_count = p_audio_chain->u32_count) > MAX_NODE_COUNT)
		return -1;

	if (!(p_audio_node_vec = calloc(1, u32_count * sizeof(AudioNode_t *))))
		return -1;

	step_to_head_node(p_audio_chain);

	for (i = 0; i < u32_count; i++) {
		if (!(p_audio_node_vec[i] = p_audio_chain->p_cur))
			return -1;

		if (step_to_next_node(p_audio_chain) != 0)
			break;
	}

	for (i = 0; i < u32_count; i++) {
		if (free_audio_node(p_audio_node_vec[i]))
			break;
	}

	free_audio_chain(p_audio_chain);

	return 0;
}

int
append_new_node(AudioChain_t *p_audio_chain)
{
	AudioNode_t *p_audio_node;

	double   f64_sampling_rate;
	uint32_t u32_mode, u32_buffer_size;

	if (!p_audio_chain)
		return -1;

	if (!p_audio_chain->p_head || !p_audio_chain->p_cur || !p_audio_chain->p_tail)
		return -1;

	if (p_audio_chain->u32_count >= MAX_NODE_COUNT)
		return -1;

	u32_mode          = p_audio_chain->p_tail->u32_mode;
	u32_buffer_size   = p_audio_chain->p_tail->u32_buffer_size;
	f64_sampling_rate = p_audio_chain->p_tail->f64_sampling_rate;

	if (!(p_audio_node = new_audio_node(f64_sampling_rate, u32_mode, u32_buffer_size)))
		goto error;

	if (append_node_to_chain(p_audio_chain, p_audio_node))
		goto error;

	return 0;

error:

	free_audio_node(p_audio_node);

	return -1;
}

int
remove_last_node(AudioChain_t *p_audio_chain)
{
	AudioNode_t *p_new_tail_node, *p_old_tail_node;
	uint32_t    u32_count;

	if (!p_audio_chain)
		return -1;

	if (!p_audio_chain->p_head || !p_audio_chain->p_cur || !p_audio_chain->p_tail)
		return -1;

	if ((u32_count = p_audio_chain->u32_count) == 0)
		return -1;

	p_old_tail_node = p_audio_chain->p_tail;

	if (!(p_new_tail_node = p_old_tail_node->p_prev))
		return -1;

	if (free_audio_node(p_old_tail_node))
		return -1;

	p_new_tail_node->p_next = NULL;

	p_audio_chain->u32_count--;

	return 0;
}

int insert_new_node_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos)
{
	AudioNode_t *p_audio_node, *p_node_at_pos;

	double f64_sampling_rate;
	uint32_t u32_mode, u32_buffer_size;

	if (!p_audio_chain)
		return -1;

	if (!p_audio_chain->p_head || !p_audio_chain->p_cur || !p_audio_chain->p_tail)
		return -1;

	if ((u32_pos > p_audio_chain->u32_count) || (p_audio_chain->u32_count >= MAX_NODE_COUNT))
		return -1;

	if (!(p_node_at_pos = get_node_by_pos(p_audio_chain, u32_pos)))
		return -1;

	u32_mode          = p_node_at_pos->u32_mode;
	u32_buffer_size   = p_node_at_pos->u32_buffer_size;
	f64_sampling_rate = p_node_at_pos->f64_sampling_rate;

	if (!(p_audio_node = new_audio_node(f64_sampling_rate, u32_mode, u32_buffer_size)))
		return -1;

	p_audio_node->p_prev = p_node_at_pos->p_prev;
	p_audio_node->p_next = p_node_at_pos;
	p_node_at_pos->p_prev = p_audio_node;

	p_audio_chain->u32_count++;

	return 0;
}

int remove_node_from_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos)
{
	if (!p_audio_chain)
		return -1;

	return 0;
}

int
load_plugin_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos, char *str_plugin_path)
{
	AudioNode_t *p_audio_node;

	if (!p_audio_chain || !str_plugin_path)
		return -1;

	if (u32_pos >= p_audio_chain->u32_count)
		return -1;

	if (!(p_audio_node = get_node_by_pos(p_audio_chain, u32_pos)))
		return -1;

	if (plugin_load(p_audio_node, str_plugin_path))
		return -1;

	return 0;
}

int
unload_plugin_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos)
{
	AudioNode_t *p_audio_node;

	if (!p_audio_chain)
		return -1;

	if (u32_pos >= p_audio_chain->u32_count)
		return -1;

	if (!(p_audio_node = get_node_by_pos(p_audio_chain, u32_pos)))
		return -1;

	if (plugin_unload(p_audio_node))
		return -1;

	return 0;
}

int
set_plugin_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos, uint32_t u32_param, void *p_value)
{
	AudioNode_t *p_audio_node;

	if (!p_audio_chain || !p_value)
		return -1;

	if (u32_pos >= p_audio_chain->u32_count)
		return -1;

	if (!(p_audio_node = get_node_by_pos(p_audio_chain, u32_pos)))
		return -1;

	if (plugin_set_param(p_audio_node, u32_param, p_value))
		return -1;

	return 0;
}

int
get_plugin_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos, uint32_t u32_param, void *p_value)
{
	AudioNode_t *p_audio_node;

	if (!p_audio_chain || !p_value)
		return -1;

	if (u32_pos >= p_audio_chain->u32_count)
		return -1;

	if (!(p_audio_node = get_node_by_pos(p_audio_chain, u32_pos)))
		return -1;

	if (plugin_get_param(p_audio_node, u32_param, p_value))
		return -1;

	return 0;
}

int
process_audio_chain(AudioChain_t *p_audio_chain, q31_t *pq31_scr, q31_t *pq31_dst)
{
	AudioNode_t *p_audio_node = NULL;
	q31_t       *pq31_process = NULL;

	if (!p_audio_chain || !pq31_scr || !pq31_dst)
		return -1;

	pq31_process = pq31_scr;

	if (step_to_head_node(p_audio_chain))
		goto skip;

	do {
		if (!(p_audio_node = p_audio_chain->p_cur))
			break;

		if (plugin_process(p_audio_node, pq31_process))
			return -1;

		pq31_process = p_audio_node->pq31_buffer;
	} while (step_to_next_node(p_audio_chain) == 0);

skip:

	memcpy(pq31_dst, pq31_process, p_audio_node->u32_buffer_size * sizeof(q31_t));

	return 0;
}
