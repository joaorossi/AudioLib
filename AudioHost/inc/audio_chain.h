#ifndef AUDIO_CHAIN_H
#define AUDIO_CHAIN_H

#include "audio_node.h"

/************************************************
 *					AudioChain					*
 ************************************************/
typedef struct AudioChain {

	/* Head, tail and current node of chain */
	AudioNode_t *p_head, *p_tail, *p_cur;

	/* Node count and position of list */
	uint32_t u32_count;

} AudioChain_t;

/************************************************
 *				AudioChain API					*
 ************************************************/
AudioChain_t * new_audio_chain(void);

int free_audio_chain(AudioChain_t *p_audio_chain);

int append_node_to_chain(AudioChain_t *p_audio_chain, AudioNode_t *p_audio_node);

AudioNode_t * get_node_by_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos);

int step_to_head_node(AudioChain_t *p_audio_chain);

int step_to_tail_node(AudioChain_t *p_audio_chain);

int step_to_next_node(AudioChain_t *p_audio_chain);

int step_to_prev_node(AudioChain_t *p_audio_chain);

#endif
