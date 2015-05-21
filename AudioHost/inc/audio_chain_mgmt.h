#ifndef AUDIO_CHAIN_MGMT_H
#define AUDIO_CHAIN_MGMT_H

#include "audio_chain.h"

/************************************************
 *					Definitions					*
 ************************************************/
#define MAX_NODE_COUNT 20

/************************************************
 *				AudioChain Mgmt API				*
 ************************************************/
AudioChain_t * load_audio_chain(uint32_t u32_node_count, uint32_t u32_mode, uint32_t u32_buffer_size, double f64_sampling_rate);

int unload_audio_chain(AudioChain_t *p_audio_chain);

int append_new_node(AudioChain_t *p_audio_chain);

int remove_last_node(AudioChain_t *p_audio_chain);

int insert_new_node_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos);

int remove_node_from_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos);

int load_plugin_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos, char *str_plugin_path);

int unload_plugin_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos);

int set_plugin_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos, uint32_t u32_param, void *p_value);

int get_plugin_at_pos(AudioChain_t *p_audio_chain, uint32_t u32_pos, uint32_t u32_param, void *p_value);

int process_audio_chain(AudioChain_t *p_audio_chain, q31_t *pq31_scr, q31_t *pq31_dst);

#endif
