#include <audio_chain.h>

/************************************************
 *				AudioChain API					*
 ************************************************/
AudioChain_t *
new_audio_chain()
{
	AudioChain_t *p_audio_chain;

	if (!(p_audio_chain = calloc(1, sizeof(AudioChain_t))))
		return NULL;

	return p_audio_chain;
}

int
free_audio_chain(AudioChain_t *p_audio_chain)
{
	if (!p_audio_chain)
		return -1;

	free(p_audio_chain);

	return 0;
}

int
append_node_to_chain(AudioChain_t *p_audio_chain, AudioNode_t *p_audio_node)
{
	AudioNode_t *p_audio_node_tail;

	if (!p_audio_chain || !p_audio_node)
		return -1;

	if (p_audio_node->u32_in_chain)
		return -1;

	if (!(p_audio_node_tail = p_audio_chain->p_tail)) {

		if (!p_audio_chain->p_head && (p_audio_chain->u32_count == 0)) {
			p_audio_chain->p_head = p_audio_node;
			p_audio_chain->p_tail = p_audio_node;
			p_audio_chain->p_cur  = p_audio_node;
			p_audio_chain->u32_count++;

			p_audio_node->p_next = NULL;
			p_audio_node->p_prev = NULL;

			return 0;
		}

		return -1;
	}

	switch (p_audio_node_tail->u32_mode) {
		case MONO_TO_MONO:
			if (p_audio_node->u32_mode == STEREO_TO_STEREO)
				return -1;
			break;

		case STEREO_TO_STEREO:
		case MONO_TO_STEREO:
			if (p_audio_node->u32_mode == MONO_TO_MONO || p_audio_node->u32_mode == MONO_TO_STEREO)
				return -1;
			break;

		default:
			return -1;
	}

	if ((p_audio_node_tail->f64_sampling_rate != p_audio_node->f64_sampling_rate) ||
		(p_audio_node_tail->u32_buffer_size   != p_audio_node->u32_buffer_size))
		return -1;

	p_audio_node_tail->p_next = p_audio_node;
	p_audio_chain->p_tail = p_audio_node;
	p_audio_chain->u32_count++;

	p_audio_node->p_prev = p_audio_node_tail;
	p_audio_node->p_next = NULL;

	return 0;
}

AudioNode_t *
get_node_by_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos)
{
	if (!p_audio_chain)
		return NULL;

	if (!p_audio_chain->p_head || !p_audio_chain->p_tail)
		return NULL;

	if (u32_pos >= p_audio_chain->u32_count)
		return NULL;

	uint32_t u32_count = 0;

	AudioNode_t *p_it;

	for (p_it = p_audio_chain->p_head; p_it != p_audio_chain->p_tail ; p_it = p_it->p_next) {
		if (u32_pos == u32_count) break;
		u32_count++;
	}

	return p_it;
}

int
step_to_head_node(AudioChain_t *p_audio_chain)
{
	if (!p_audio_chain)
		return -1;

	if (!p_audio_chain->p_head || !p_audio_chain->p_cur || !p_audio_chain->p_tail)
		return -1;

	p_audio_chain->p_cur = p_audio_chain->p_head;

	return 0;
}

int
step_to_tail_node(AudioChain_t *p_audio_chain)
{
	if (!p_audio_chain)
		return -1;

	if (!p_audio_chain->p_head || !p_audio_chain->p_cur || !p_audio_chain->p_tail)
		return -1;

	p_audio_chain->p_cur = p_audio_chain->p_tail;

	return 0;
}

int
step_to_next_node(AudioChain_t *p_audio_chain)
{
	if (!p_audio_chain)
		return -1;

	if (!p_audio_chain->p_head || !p_audio_chain->p_cur || !p_audio_chain->p_tail)
		return -1;

	if (!p_audio_chain->p_cur->p_next)
		return -1;

	p_audio_chain->p_cur = p_audio_chain->p_cur->p_next;

	return 0;
}

int
step_to_prev_node(AudioChain_t *p_audio_chain)
{
	if (!p_audio_chain)
		return -1;

	if (!p_audio_chain->p_head || !p_audio_chain->p_cur || !p_audio_chain->p_tail)
		return -1;

	if (!p_audio_chain->p_cur->p_prev)
		return -1;

	p_audio_chain->p_cur = p_audio_chain->p_cur->p_prev;

	return 0;
}
