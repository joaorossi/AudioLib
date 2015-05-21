#include <audio_node.h>
#include <dlfcn.h>

/************************************************
 *					PluginSlot					*
 ************************************************/
typedef struct PluginSlot {

	/* Pointer to plugin handle */
	void     *p_plugin;

	/* Pointer to lib handle */
	void     *p_plugin_handler;

	/* Plugin access functions */
	void * (*init)(double);
	int    (*deinit)(void *);
	int    (*set_param)(void *, int, void *);
	int    (*get_param)(void *, int, void *);
	int    (*process)(void *, q31_t *, q31_t *, uint32_t);

} PluginSlot_t;

/************************************************
 *				Static Functions				*
 ************************************************/
static void
_pass_thru(uint32_t u32_mode, q31_t *pq31_src, q31_t *pq31_dst, uint32_t u32_buffer_size)
{
	if (u32_mode == MONO_TO_MONO)
		memcpy(pq31_dst, pq31_src, u32_buffer_size * sizeof(q31_t));
	else if (u32_mode == STEREO_TO_STEREO)
		memcpy(pq31_dst, pq31_src, 2 * u32_buffer_size * sizeof(q31_t));
	else if (u32_mode == MONO_TO_STEREO)
		do {*pq31_dst++ = *pq31_src; *pq31_dst++ = *pq31_src++;} while (--u32_buffer_size);
}

/************************************************
 *					AudioNode API				*
 ************************************************/
AudioNode_t *
new_audio_node(double f64_sampling_rate, uint32_t u32_mode, uint32_t u32_buffer_size)
{
	AudioNode_t  *p_audio_node  = NULL;
	PluginSlot_t *p_plugin_slot = NULL;
	q31_t        *pq31_buffer   = NULL;

	int32_t i32_num_of_chan;

	/* Check buffer size */
	if (u32_buffer_size > MAX_BUFFER_SIZE)
		goto error;

	if (f64_sampling_rate > MAX_SAMPLING_RATE)
		goto error;

	if (u32_mode == MONO_TO_MONO)
		i32_num_of_chan = 1;
	else if (u32_mode == STEREO_TO_STEREO || u32_mode == MONO_TO_STEREO)
		i32_num_of_chan = 2;
	else
		goto error;

	/* Allocate memory for node */
	if (!(p_audio_node  = calloc(1, sizeof(AudioNode_t)))  ||
		!(p_plugin_slot = calloc(1, sizeof(PluginSlot_t))) ||
		!(pq31_buffer   = calloc(1, i32_num_of_chan * u32_buffer_size * sizeof(q31_t))))
		goto error;

	/* Fill node structure */
	p_audio_node->p_plugin_slot     = p_plugin_slot;
	p_audio_node->pq31_buffer       = pq31_buffer;
	p_audio_node->u32_buffer_size   = u32_buffer_size;
	p_audio_node->u32_mode          = u32_mode;
	p_audio_node->f64_sampling_rate = f64_sampling_rate;

	return p_audio_node;

error:

	if (p_audio_node)
		free(p_audio_node);

	if (p_plugin_slot)
		free(p_plugin_slot);

	if (pq31_buffer)
		free(pq31_buffer);

	return NULL;
}

int
free_audio_node(AudioNode_t *p_audio_node)
{
	PluginSlot_t *p_plugin_slot;

	if (!p_audio_node)
		return -1;

	if (p_audio_node->p_plugin_slot) {
		p_plugin_slot = p_audio_node->p_plugin_slot;

		if (p_plugin_slot->p_plugin_handler || p_plugin_slot->p_plugin)
			return -1;

		free(p_audio_node->p_plugin_slot);
	}

	if (p_audio_node->pq31_buffer)
		free(p_audio_node->pq31_buffer);

	free(p_audio_node);

	return 0;
}

int
plugin_load(AudioNode_t *p_audio_node, char *str_plugin_path)
{
	PluginSlot_t *p_plugin_slot = NULL;

	/* Check for inconsistency */
	if (!p_audio_node || !str_plugin_path)
		return -1;

	if (!(p_plugin_slot = p_audio_node->p_plugin_slot))
		return -1;

	if (p_plugin_slot->p_plugin || p_plugin_slot->p_plugin_handler)
		return -1;

	/* Load plugin */
	p_plugin_slot->p_plugin_handler = dlopen(str_plugin_path, RTLD_NOW);

	if (dlerror())
		goto error;

	/* Resolve plugin symbols */
	p_plugin_slot->init      = dlsym(p_plugin_slot->p_plugin_handler, "plugin_init");
	p_plugin_slot->deinit    = dlsym(p_plugin_slot->p_plugin_handler, "plugin_deinit");
	p_plugin_slot->set_param = dlsym(p_plugin_slot->p_plugin_handler, "plugin_set_param");
	p_plugin_slot->get_param = dlsym(p_plugin_slot->p_plugin_handler, "plugin_get_param");

	switch (p_audio_node->u32_mode) {
		case MONO_TO_MONO:
			p_plugin_slot->process = dlsym(p_plugin_slot->p_plugin_handler,
										   "plugin_process__MONO_IN_MONO_OUT");
			break;

		case MONO_TO_STEREO:
			p_plugin_slot->process = dlsym(p_plugin_slot->p_plugin_handler,
										   "plugin_process__MONO_IN_STEREO_OUT");
			break;

		case STEREO_TO_STEREO:
			p_plugin_slot->process = dlsym(p_plugin_slot->p_plugin_handler,
										   "plugin_process__STEREO_IN_STEREO_OUT");
			break;

		default:
			goto error;
	}

	if (dlerror())
		goto error;

	/* Init the plugin */
	if (!(p_plugin_slot->p_plugin = p_plugin_slot->init(p_audio_node->f64_sampling_rate)))
		goto error;

	return 0;

error:

	if (p_plugin_slot->p_plugin_handler) {

		if (p_plugin_slot->p_plugin)
			p_plugin_slot->deinit(p_plugin_slot->p_plugin);

		dlclose(p_plugin_slot->p_plugin_handler);

		p_plugin_slot->p_plugin_handler = NULL;
		p_plugin_slot->p_plugin         = NULL;
		p_plugin_slot->init             = NULL;
		p_plugin_slot->deinit           = NULL;
		p_plugin_slot->set_param        = NULL;
		p_plugin_slot->get_param        = NULL;
		p_plugin_slot->process          = NULL;
	}

	return -1;
}

int
plugin_unload(AudioNode_t *p_audio_node)
{
	PluginSlot_t *p_plugin_slot;

	if (!p_audio_node)
		return -1;

	if (!(p_plugin_slot = p_audio_node->p_plugin_slot))
		return -1;

	if (p_plugin_slot->p_plugin_handler) {

		if (p_plugin_slot->p_plugin)
			p_plugin_slot->deinit(p_plugin_slot->p_plugin);

		dlclose(p_plugin_slot->p_plugin_handler);

		p_plugin_slot->p_plugin_handler = NULL;
		p_plugin_slot->p_plugin         = NULL;
		p_plugin_slot->init             = NULL;
		p_plugin_slot->deinit           = NULL;
		p_plugin_slot->set_param        = NULL;
		p_plugin_slot->get_param        = NULL;
		p_plugin_slot->process          = NULL;
	}

	return 0;
}

int
plugin_set_param(AudioNode_t *p_audio_node, uint32_t u32_param, void *p_value)
{
	PluginSlot_t *p_plugin_slot;

	if (!p_audio_node || !p_value)
		return -1;

	if (!(p_plugin_slot = p_audio_node->p_plugin_slot))
		return -1;

	if (!p_plugin_slot->p_plugin_handler || !p_plugin_slot->p_plugin)
		return -1;

	return p_plugin_slot->set_param(p_plugin_slot->p_plugin, u32_param, p_value);
}

int
plugin_get_param(AudioNode_t *p_audio_node, uint32_t u32_param, void *p_value)
{
	PluginSlot_t *p_plugin_slot;

	if (!p_audio_node || !p_value)
		return -1;

	if (!(p_plugin_slot = p_audio_node->p_plugin_slot))
		return -1;

	if (!p_plugin_slot->p_plugin_handler || !p_plugin_slot->p_plugin)
		return -1;

	return p_plugin_slot->get_param(p_plugin_slot->p_plugin, u32_param, p_value);
}

int
plugin_process(AudioNode_t *p_audio_node, q31_t *pq31_in_buffer)
{
	PluginSlot_t *p_plugin_slot;

	if (!p_audio_node || !pq31_in_buffer)
		return -1;

	if (!p_audio_node->pq31_buffer)
		return -1;

	if (!(p_plugin_slot = p_audio_node->p_plugin_slot))
		return -1;

	if (!p_plugin_slot->p_plugin_handler || !p_plugin_slot->p_plugin) {
		_pass_thru(p_audio_node->u32_mode, pq31_in_buffer,
				   p_audio_node->pq31_buffer, p_audio_node->u32_buffer_size);
		return 0;
	}

	return p_plugin_slot->process(p_plugin_slot->p_plugin,
								  pq31_in_buffer, p_audio_node->pq31_buffer,
								  p_audio_node->u32_buffer_size);
}
