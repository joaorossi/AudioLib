#include <testunit.h>

int
filter_ctrl(struct AudioInfo *p_audio_info)
{
	if (p_audio_info == NULL)
		return -1;

	char str_input[MAX_PARAM_LEN];
	char *str_freq = "freq", *str_gain = "gain", *str_reso = "reso", *str_type = "type",
		 *str_bypass_on = "bypass_on", *str_bypass_off = "bypass_off", *str_exit = "exit",
		 *str_lpf6db = "lpf_6db", *str_lpf12db = "lpf_12db",
		 *str_hpf6db = "hpf_6db", *str_hpf12db = "hpf_12db",
		 *str_param = "param";

	double   f64_value;
	uint32_t u32_bypass, u32_filt_type;

	while (1) {
		printf("Enter parameter name:\n");
		scanf("%s", str_input);

		if (strncmp(str_input, str_bypass_on, MAX_PARAM_LEN) == 0) {
			u32_bypass = 1;
			if (plugin_set_param(p_audio_info->p_plugin, 0, &u32_bypass))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_bypass_off, MAX_PARAM_LEN) == 0) {
			u32_bypass = 0;
			if (plugin_set_param(p_audio_info->p_plugin, 0, &u32_bypass))
				return -1;
			continue;
		}

		if (strncmp(str_freq, str_input, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter freq value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 1, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_reso, str_input, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter reso value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 2, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_gain, str_input, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 3, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_type, str_input, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter filter type:\n");
			scanf("%s", str_input);

			if (strcmp(str_lpf6db, str_input) == 0)
				u32_filt_type = 0;
			else if (strcmp(str_lpf12db, str_input) == 0)
				u32_filt_type = 1;
			else if (strcmp(str_hpf6db, str_input) == 0)
				u32_filt_type = 5;
			else if (strcmp(str_hpf12db, str_input) == 0)
				u32_filt_type = 6;
			else if (strcmp(str_param, str_input) == 0)
				u32_filt_type = 14;
			else
				return -1;

			if (plugin_set_param(p_audio_info->p_plugin, 4, &u32_filt_type))
				return -1;

			continue;
		}

		if (strncmp(str_exit, str_input, MAX_PARAM_LEN) == 0) {
			return 0;
		}

		fprintf(stdout, "Unknown parameter.\n");
	}
}

int
graphiceq_ctrl(struct AudioInfo *p_audio_info)
{
	if (p_audio_info == NULL)
		return -1;

	char str_input[MAX_PARAM_LEN], *str_exit = "exit",
		 *str_bypass_on  = "bypass_on",  *str_bypass_off = "bypass_off",
		 *str_gain_band1 = "gain_band1", *str_gain_band2 = "gain_band2",
		 *str_gain_band3 = "gain_band3", *str_gain_band4 = "gain_band4",
		 *str_gain_band5 = "gain_band5", *str_gain_band6 = "gain_band6",
		 *str_gain_band7 = "gain_band7", *str_gain_band8 = "gain_band8",
		 *str_gain_band9 = "gain_band9", *str_gain_band10 = "gain_band10";

	double   f64_value;
	uint32_t u32_bypass;

	while (1) {
		fprintf(stdout, "Enter parameter name:\n");
		scanf("%s", str_input);

		if (strncmp(str_input, str_bypass_on, MAX_PARAM_LEN) == 0) {
			u32_bypass = 1;
			if (plugin_set_param(p_audio_info->p_plugin, 0, &u32_bypass))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_bypass_off, MAX_PARAM_LEN) == 0) {
			u32_bypass = 0;
			if (plugin_set_param(p_audio_info->p_plugin, 0, &u32_bypass))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band1, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 1 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 1, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band2, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 2 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 2, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band3, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 3 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 3, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band4, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 4 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 4, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band5, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 5 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 5, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band6, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 6 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 6, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band7, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 7 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 7, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band8, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 8 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 8, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band9, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 9 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 9, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_gain_band10, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter band 10 gain value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 10, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_exit, MAX_PARAM_LEN) == 0)
			return 0;

		fprintf(stderr, "Unknown parameter.\n");
	}
}

int
delay_ctrl(struct AudioInfo *p_audio_info)
{
	if (p_audio_info == NULL)
		return -1;

	char str_input[MAX_PARAM_LEN], *str_exit = "exit",
		 *str_bypass_on = "bypass_on", *str_bypass_off = "bypass_off",
		 *str_time = "time", *str_feedback = "feedback", *str_mix = "mix";

	uint32_t u32_bypass;
	double   f64_value;

	while (1) {
		fprintf(stdout, "Enter parameter name:\n");
		scanf("%s", str_input);

		if (strncmp(str_input, str_bypass_on, MAX_PARAM_LEN) == 0) {
			u32_bypass = 1;
			if (plugin_set_param(p_audio_info->p_plugin, 0, &u32_bypass))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_bypass_off, MAX_PARAM_LEN) == 0) {
			u32_bypass = 0;
			if (plugin_set_param(p_audio_info->p_plugin, 0, &u32_bypass))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_time, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter delay time value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 1, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_feedback, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter feedback percentage value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 2, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_mix, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter mix percentage value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 3, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_exit, MAX_PARAM_LEN) == 0)
			return 0;

		fprintf(stderr, "Unknown parameter.\n");
	}

	return 0;
}

int
flanger_ctrl(struct AudioInfo *p_audio_info)
{
	if (p_audio_info == NULL)
		return -1;

	char str_input[MAX_PARAM_LEN], *str_exit = "exit",
		 *str_bypass_on = "bypass_on", *str_bypass_off = "bypass_off",
		 *str_rate = "rate", *str_feedback = "feedback", *str_depth = "depth",
		 *str_wave = "wave", *str_stereo = "mode";

	char *str_wave_sin = "sin", *str_wave_tri = "tri", *str_wave_saw = "saw", *str_wave_sqr = "sqr";

	char *str_stereo_on = "stereo", *str_stereo_off = "mono";

	uint32_t u32_bypass, u32_wave, u32_stereo;
	double   f64_value;

	while (1) {
		fprintf(stdout, "Enter parameter name:\n");
		scanf("%s", str_input);

		if (strncmp(str_input, str_bypass_on, MAX_PARAM_LEN) == 0) {
			u32_bypass = 1;
			if (plugin_set_param(p_audio_info->p_plugin, 0, &u32_bypass))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_bypass_off, MAX_PARAM_LEN) == 0) {
			u32_bypass = 0;
			if (plugin_set_param(p_audio_info->p_plugin, 0, &u32_bypass))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_rate, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter rate value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 1, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_feedback, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter feedback value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 2, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_depth, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter depth value:\n");
			scanf("%lf", &f64_value);
			if (plugin_set_param(p_audio_info->p_plugin, 3, &f64_value))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_wave, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter wave form:\n");
			scanf("%s", str_input);

			if (strncmp(str_input, str_wave_sin, 3) == 0)
				u32_wave = 0;
			else if (strncmp(str_input, str_wave_tri, 3) == 0)
				u32_wave = 1;
			else if (strncmp(str_input, str_wave_sqr, 3) == 0)
				u32_wave = 2;
			else if (strncmp(str_input, str_wave_saw, 3) == 0)
				u32_wave = 3;
			else
				continue;

			if (plugin_set_param(p_audio_info->p_plugin, 4, &u32_wave))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_stereo, MAX_PARAM_LEN) == 0) {
			fprintf(stdout, "Enter mode:\n");
			scanf("%s", str_input);

			if (strncmp(str_input, str_stereo_on, MAX_PARAM_LEN) == 0)
				u32_stereo = 1;
			else if (strncmp(str_input, str_stereo_off, MAX_PARAM_LEN) == 0)
				u32_stereo = 0;
			else
				continue;

			if (plugin_set_param(p_audio_info->p_plugin, 5, &u32_stereo))
				return -1;
			continue;
		}

		if (strncmp(str_input, str_exit, MAX_PARAM_LEN) == 0)
			return 0;

		fprintf(stderr, "Unknown parameter.\n");
	}

	return 0;
}
