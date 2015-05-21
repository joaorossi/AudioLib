#include <audio_chain.h>
#include <audio_node.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{

	AudioChain_t *p_audio_chain = NULL;
	AudioNode_t  **p_audio_node_vec = NULL;

	AudioNode_t *p_current_node;

	int ret = 0, count = 0, num_of_nodes = 0, i;

	if (argc < 2) {
		fprintf(stdout, "Usage: AudioChainTest [Num of Nodes]\n");
		return -1;
	}

	num_of_nodes = atoi(argv[1]);
	if ((num_of_nodes > 10000) || (num_of_nodes < 0)) {
		fprintf(stderr, "Invalid number pf nodes.\n");
		return -1;
	}

	q31_t *pq31_proof = calloc(1, MAX_BUFFER_SIZE * sizeof(q31_t));
	for (i = 0; i < MAX_BUFFER_SIZE; i++) pq31_proof[i] = i;

	if (!(p_audio_chain = new_audio_chain())) {
		fprintf(stderr, "Failed allcating audio chain.\n");
		ret = -1;
		goto cleanup;
	}

	if (!(p_audio_node_vec = calloc(1, num_of_nodes * sizeof(AudioNode_t *)))) {
		fprintf(stderr, "Failed allocating audio node vector.\n");
		ret = -1;
		goto cleanup;
	}

	for (i = 0; i < num_of_nodes; i++) {
		if (!(p_audio_node_vec[i] = new_audio_node(MAX_SAMPLING_RATE, MONO_TO_MONO, MAX_BUFFER_SIZE))) {
			fprintf(stderr, "Failed allocating audio node #%d\n", i);
			ret = -1;
			goto cleanup;
		}

		if (append_node_to_tail(p_audio_chain, p_audio_node_vec[i])) {
			fprintf(stderr, "Failed appending audio node #%d\n", i);
			ret = -1;
			goto cleanup;
		}
	}

	fprintf(stdout, "Nodes appended to chain: %d\n", p_audio_chain->u32_count);

	step_to_head_node(p_audio_chain);

	do {

		count++;
		fprintf(stdout, "Processing node: %d\n", count);

		p_current_node = p_audio_chain->p_cur;

		plugin_process(p_current_node, pq31_proof);

		for (i = 0; i < MAX_BUFFER_SIZE; i++) {
			if (pq31_proof[i] != p_current_node->pq31_buffer[i]) {
				fprintf(stdout, "Failed at node %d, count: %d\n", count, i);
				goto cleanup;
			}
		}

		pq31_proof = p_current_node->pq31_buffer;

	} while (step_to_next_node(p_audio_chain) == 0);

	for (i = 0; i < MAX_BUFFER_SIZE; i++) {
		if (pq31_proof[i] != i) {
			fprintf(stdout, "Failed at: %d\n", i);
			goto cleanup;
		}
	}

	fprintf(stdout, "Test scceeded!\n");

cleanup:

	if (free_audio_chain(p_audio_chain))
		fprintf(stderr, "Failed freeing audio chain.\n");

	for (i = 0; i < num_of_nodes; i++) {
		if (free_audio_node(p_audio_node_vec[i]))
			fprintf(stderr, "Failed freeing audio node #%d\n", i);
	}

	return ret;
}