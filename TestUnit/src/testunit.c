#include <testunit.h>
#include <dlfcn.h>
#include <portaudio.h>

/****************************************************************
 *					Dynamic Libraries Pointers					*
 ****************************************************************/

void *p_plugin_handler;

void * (*plugin_init)                          (double);
int    (*plugin_deinit)                        (void *);
int    (*plugin_set_param)                     (void *, int, void *);
int    (*plugin_get_param)                     (void *, int , void *);
int    (*plugin_process__MONO_IN_MONO_OUT)     (void *, q31_t *, q31_t *, uint32_t);
int    (*plugin_process__MONO_IN_STEREO_OUT)   (void *, q31_t *, q31_t *, uint32_t);
int    (*plugin_process__STEREO_IN_STEREO_OUT) (void *, q31_t *, q31_t *, uint32_t);

/****************************************************************
 *						Audio Callback 							*
 ****************************************************************/

static int
audiocb(const void *p_input_buffer,
		void *p_output_buffer,
		unsigned long u32_buffer_size,
		const PaStreamCallbackTimeInfo *pa_time_info,
		PaStreamCallbackFlags pa_status_flags,
		void *p_user_data)
{
	if ((p_user_data == NULL) || (p_plugin_handler == NULL)) {
		fprintf(stderr, "Error: Null user data pointer at callback.\n");
		return paAbort;
	}

	int32_t i32_ret = -1;

	/* Cast buffers */
	q31_t *pq31_input  = (q31_t *) p_input_buffer;
	q31_t *pq31_output = (q31_t *) p_output_buffer;

	/* Cast audio info pointer */
	struct AudioInfo *p_audio_info = p_user_data;

	/* Audio channels mode */
	switch (p_audio_info->u32_mode) {
		case MONO_IN__MONO_OUT:
			i32_ret = plugin_process__MONO_IN_MONO_OUT(p_audio_info->p_plugin,
													   pq31_input, pq31_output,
													   u32_buffer_size);
			break;

		case MONO_IN__STEREO_OUT:
			i32_ret = plugin_process__MONO_IN_STEREO_OUT(p_audio_info->p_plugin,
														 pq31_input, pq31_output,
														 u32_buffer_size);
			break;

		case STEREO_IN__STEREO_OUT:
			i32_ret = plugin_process__STEREO_IN_STEREO_OUT(p_audio_info->p_plugin,
														   pq31_input, pq31_output,
														   u32_buffer_size);
			break;

		default:
			fprintf(stderr, "Error: Invalid channel count at callback.\n");
			return paAbort;
	}

	if (i32_ret) {
		fprintf(stderr, "Error: Callback failed to process buffer.\n");
		return paAbort;
	}

	return paContinue;
}

/****************************************************************
 *						Argument Parser							*
 ****************************************************************/

static int
_parse_args(int argc, char **argv, struct AudioInfo *p_audio_info)
{
	if ((argv == NULL) || (p_audio_info == NULL)) {
		fprintf(stderr, "Error: Internal error!!!\n");
		return -1;
	}

	/* Audio config args */
	char *str_mono   = "-m";
	char *str_stereo = "-s";
	char *str_48k    = "-48k";
	char *str_96k    = "-96k";
	char *str_64     = "-64";
	char *str_128    = "-128";
	char *str_256    = "-256";

	/* Check for correct arg count */
	if (argc != 5) {
		fprintf(stderr, "Error: Invalid argument count!\n");
		return -1;
	}

	/* Isolate args */
	char *str_in_ch_cnt     = argv[1];
	char *str_out_ch_cnt    = argv[2];
	char *str_sampling_rate = argv[3];
	char *str_buffer_size   = argv[4];

	/* Parse input channel count arg */
	if (strcmp(str_in_ch_cnt, str_mono) == 0) {
		p_audio_info->u32_in_ch_cnt = 1;
		fprintf(stdout, "  INPUT: MONO\n\n");
	} else if (strcmp(str_in_ch_cnt, str_stereo) == 0) {
		p_audio_info->u32_in_ch_cnt = 2;
		fprintf(stdout, "  INPUT: STEREO\n\n");
	} else {
		fprintf(stderr, "  INPUT: Channel count error!\n\n");
		return -1;
	}

	/* Parse output channel count arg */
	if (strcmp(str_out_ch_cnt, str_mono) == 0) {
		p_audio_info->u32_out_ch_cnt = 1;
		fprintf(stdout, "  OUTPUT: MONO\n\n");
	} else if (strcmp(str_out_ch_cnt, str_stereo) == 0) {
		p_audio_info->u32_out_ch_cnt = 2;
		fprintf(stdout, "  OUTPUT: STEREO\n\n");
	} else {
		fprintf(stderr, "  OUTPUT: Channel count error!\n\n");
		return -1;
	}

	/* Parse samplerate arg */
	if (strcmp(str_sampling_rate, str_48k) == 0) {
		p_audio_info->f64_sampling_rate = 48000.0;
		fprintf(stdout, "  SAMPLE RATE: 48kHz\n\n");
	} else if (strcmp(str_sampling_rate, str_96k) == 0) {
		p_audio_info->f64_sampling_rate = 96000.0;
		fprintf(stdout, "  SAMPLE RATE: 96kHz\n\n");
	} else {
		fprintf(stderr, "  SAMPLE RATE: Error!\n\n");
		return -1;
	}

	/* Parse buffer size arg */
	if (strcmp(str_buffer_size, str_64) == 0) {
		p_audio_info->u32_buffer_size = 64;
		fprintf(stdout, "  BUFFER SIZE: 64\n\n");
	} else if (strcmp(str_buffer_size, str_128) == 0) {
		p_audio_info->u32_buffer_size = 128;
		fprintf(stdout, "  BUFFER SIZE: 128\n\n");
	} else if (strcmp(str_buffer_size, str_256) == 0) {
		p_audio_info->u32_buffer_size = 256;
		fprintf(stdout, "  BUFFER SIZE: 256\n\n");
	} else {
		fprintf(stderr, "  BUFFER SIZE: Error!\n\n");
		return -1;
	}

	/* Channel routing parameters */
	if ((p_audio_info->u32_in_ch_cnt == 1) && (p_audio_info->u32_out_ch_cnt == 1))
		p_audio_info->u32_mode = MONO_IN__MONO_OUT;

	if ((p_audio_info->u32_in_ch_cnt == 1) && (p_audio_info->u32_out_ch_cnt == 2))
		p_audio_info->u32_mode = MONO_IN__STEREO_OUT;

	if ((p_audio_info->u32_in_ch_cnt == 2) && (p_audio_info->u32_out_ch_cnt == 2))
		p_audio_info->u32_mode = STEREO_IN__STEREO_OUT;

	return 0;
}

/****************************************************************
 *							Plugin Mgmt 						*
 ****************************************************************/
static int
_load_plugin(struct AudioInfo *p_audio_info)
{
	if (p_audio_info == NULL)
		return -1;

	char *str_filter    = "Filter";
	char *str_graphiceq = "GraphicEQ";
	char *str_delay     = "Delay";
	char *str_flanger   = "Flanger";
	char str_input[MAX_PARAM_LEN];
	char *str_plugin_path;
	char *str_dlerror;

	/* Get plugin name from user */
	fprintf(stdout, "Plugin name:\n");
	scanf("%s", str_input);

	/* Parse selected plugin */
	if (strcmp(str_input, str_filter) == 0) {
		p_audio_info->u32_plugin_id = FILTER_PLUGIN;
		str_plugin_path = FILTER_PLUGIN_PATH;
	} else if (strcmp(str_input, str_graphiceq) == 0) {
		p_audio_info->u32_plugin_id = GRAPHICEQ_PLUGIN;
		str_plugin_path = GRAPHICEQ_PLUGIN_PATH;
	} else if (strcmp(str_input, str_delay) == 0) {
		p_audio_info->u32_plugin_id = DELAY_PLUGIN;
		str_plugin_path = DELAY_PLUGIN_PATH;
	} else if (strcmp(str_input, str_flanger) == 0) {
		p_audio_info->u32_plugin_id = FLANGER_PLUGIN;
		str_plugin_path = FLANGER_PLUGIN_PATH;
	} else {
		p_audio_info->u32_plugin_id = NONE_PLUGIN;
		fprintf(stderr, "Error: No plugin selected!\n");
		return -1;
	}

	/* Load dynamic lib */
	p_plugin_handler = dlopen(str_plugin_path, RTLD_NOW);

	if ((str_dlerror = dlerror())) {
		fprintf(stderr, "Error: %s.\n", str_dlerror);
		return -1;
	}

	plugin_init                          = dlsym(p_plugin_handler, "plugin_init");
	plugin_deinit                        = dlsym(p_plugin_handler, "plugin_deinit");
	plugin_set_param                     = dlsym(p_plugin_handler, "plugin_set_param");
	plugin_get_param                     = dlsym(p_plugin_handler, "plugin_get_param");
	plugin_process__MONO_IN_MONO_OUT     = dlsym(p_plugin_handler, "plugin_process__MONO_IN_MONO_OUT");
	plugin_process__MONO_IN_STEREO_OUT   = dlsym(p_plugin_handler, "plugin_process__MONO_IN_STEREO_OUT");
	plugin_process__STEREO_IN_STEREO_OUT = dlsym(p_plugin_handler, "plugin_process__STEREO_IN_STEREO_OUT");

	if ((str_dlerror = dlerror())) {
		fprintf(stderr, "Error: %s.\n", str_dlerror);
		dlclose(p_plugin_handler);
		return -1;
	}

	/* Allocate plugin */
	if ((p_audio_info->p_plugin = plugin_init(p_audio_info->f64_sampling_rate)) == NULL) {
		fprintf(stderr, "Error: Failed allocating plugin.\n");
		dlclose(p_plugin_handler);
		return -1;
	}

	fprintf(stdout, "%s plugin succesfully loaded!\n", str_input);

	return 0;
}

static int
_unload_plugin(struct AudioInfo *p_audio_info)
{
	if ((p_audio_info == NULL) || (p_plugin_handler == NULL))
		return -1;

	char *str_dlerror = NULL;

	if (plugin_deinit(p_audio_info->p_plugin)) {
		fprintf(stderr, "Error: Failed deinitializing plugin.\n");
		return -1;
	}

	if (dlclose(p_plugin_handler)) {
		fprintf(stderr, "%s\n", str_dlerror);
		return -1;
	}

	p_audio_info->u32_plugin_id = NONE_PLUGIN;

	return 0;
}

/****************************************************************
 *							User Control						*
 ****************************************************************/

static int
_get_cmd_ctrl(struct AudioInfo *p_audio_info)
{
	if ((p_audio_info == NULL) || (p_plugin_handler == NULL))
		return -1;

	if (p_audio_info->p_plugin == NULL)
		return -1;

	switch (p_audio_info->u32_plugin_id) {
		case FILTER_PLUGIN:
			if (filter_ctrl(p_audio_info))
				return -1;
			break;

		case GRAPHICEQ_PLUGIN:
			if (graphiceq_ctrl(p_audio_info))
				return -1;
			break;

		case DELAY_PLUGIN:
			if (delay_ctrl(p_audio_info))
				return -1;
			break;

		case FLANGER_PLUGIN:
			if (flanger_ctrl(p_audio_info))
				return -1;
			break;
	}

	return 0;
}

/****************************************************************
 *							 Main								*
 ****************************************************************/
int
main(int argc, char *argv[])
{
	PaStreamParameters pa_input_params, pa_output_params;
	PaStream *pa_stream;
	PaError pa_err = 0;
	struct AudioInfo *p_audio_info = NULL;

	/* Allocate audio info struct */
	if ((p_audio_info = calloc(1, sizeof(struct AudioInfo))) == NULL) {
		fprintf(stderr, "Error: Failed allocating audio info struct.\n");
		return -1;
	}

	/* Parse arguments */
	if (_parse_args(argc, argv, p_audio_info)) {
		fprintf(stderr, "Error: Failed parsing arguments.\n");
		goto error;
	}

	/* Load plugin */
	if (_load_plugin(p_audio_info)) {
		fprintf(stderr, "Error: Failed to load plugin.\n");
		goto error;
	}

	/* Initializing portaudio */
	fprintf(stdout, "Initializing portaudio!\n");
	if ((pa_err = Pa_Initialize()) != paNoError) {
		fprintf(stderr, "Error: Failed initializing portaudio.\n");
		goto error;
	}

	/* Setting audio input parameters */
	fprintf(stdout, "Quering audio input devices.\n");
	if ((pa_input_params.device = Pa_GetDefaultInputDevice()) == paNoDevice) {
		fprintf(stderr, "Error: Couldn't find default input device.\n");
		goto error;
	}
	pa_input_params.channelCount = p_audio_info->u32_in_ch_cnt;
	pa_input_params.sampleFormat = paInt32;
	pa_input_params.suggestedLatency = Pa_GetDeviceInfo(pa_input_params.device)->defaultLowInputLatency;
	pa_input_params.hostApiSpecificStreamInfo = NULL;

	/* Setting audio output parameters */
	fprintf(stdout, "Quering audio output devices.\n");
	if ((pa_output_params.device = Pa_GetDefaultOutputDevice()) == paNoDevice) {
		fprintf(stderr, "Error: Couldn't find default output device.\n");
		goto error;
	}
	pa_output_params.channelCount = p_audio_info->u32_out_ch_cnt;
	pa_output_params.sampleFormat = paInt32;
	pa_output_params.suggestedLatency = Pa_GetDeviceInfo(pa_output_params.device)->defaultLowOutputLatency;
	pa_output_params.hostApiSpecificStreamInfo = NULL;

	/* Opening portaudio stream */
	fprintf(stdout, "Opening portaudio stream.\n");
	pa_err = Pa_OpenStream(&pa_stream,
						   &pa_input_params,
						   &pa_output_params,
						   p_audio_info->f64_sampling_rate,
						   p_audio_info->u32_buffer_size,
						   paNoFlag,
						   audiocb,
						   p_audio_info);
	if (pa_err != paNoError) {
		fprintf(stderr, "Error: Failed opening portaudio stream.\n");
		goto error;
	}

	/* Starts portaudio stream */
	fprintf(stdout, "Starting portaudio stream!\n");
	if ((pa_err = Pa_StartStream(pa_stream)) != paNoError) {
		fprintf(stderr, "Error: Failed starting portaudio stream.\n");
		goto error;
	}

	/* Get plugin control parameters from user */
	if (_get_cmd_ctrl(p_audio_info))
		fprintf(stderr, "Invalid parameters.\n");

	/* Closes portaudio stream */
	fprintf(stdout, "Closing portaudio stream.\n");
	if ((pa_err = Pa_CloseStream(pa_stream)) != paNoError) {
		fprintf(stderr, "Error: Failed closing portaudio stream.\n");
		goto error;
	}

	Pa_Terminate();

	/* Unload plugin */
	fprintf(stdout, "Unloading plugin.\n");
	if (_unload_plugin(p_audio_info))
		fprintf(stderr, "Error: Failed to unload plugin.\n");

	/* Daeallocating audio info struct */
	free(p_audio_info);

	printf("Test unit finished!\n");

	return 0;

error:

	Pa_Terminate();
	_unload_plugin(p_audio_info->p_plugin);
	free(p_audio_info);
	printf("Portaudio message: %s\n", Pa_GetErrorText(pa_err));
	return -1;
}
