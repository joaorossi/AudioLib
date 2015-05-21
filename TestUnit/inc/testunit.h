#ifndef TESTUNIT_H
#define TESTUNIT_H

#include <stdio.h>
#include <fixedpoint.h>

#define FILTER_PLUGIN_PATH    "/Users/joaorossifilho/Documents/CronusCore/Plugins/bin/libFilter.dylib"
#define GRAPHICEQ_PLUGIN_PATH "/Users/joaorossifilho/Documents/CronusCore/Plugins/bin/libGraphicEQ.dylib"
#define DELAY_PLUGIN_PATH     "/Users/joaorossifilho/Documents/CronusCore/Plugins/bin/libDelay.dylib"
#define FLANGER_PLUGIN_PATH   "/Users/joaorossifilho/Documents/CronusCore/Plugins/bin/libFlanger.dylib"

#define MAX_PARAM_LEN 64

extern void *p_plugin_handler;

extern void * (*plugin_init)                          (double);
extern int    (*plugin_deinit)                        (void *);
extern int    (*plugin_set_param)                     (void *, int, void *);
extern int    (*plugin_get_param)                     (void *, int , void *);
extern int    (*plugin_process__MONO_IN_MONO_OUT)     (void *, q31_t *, q31_t *, uint32_t);
extern int    (*plugin_process__MONO_IN_STEREO_OUT)   (void *, q31_t *, q31_t *, uint32_t);
extern int    (*plugin_process__STEREO_IN_STEREO_OUT) (void *, q31_t *, q31_t *, uint32_t);

/* Routing mode enum */
enum {
	MONO_IN__MONO_OUT,
	MONO_IN__STEREO_OUT,
	STEREO_IN__STEREO_OUT
};

/* Plugin id enum */
enum {
	NONE_PLUGIN = 0,
	FILTER_PLUGIN,
	GRAPHICEQ_PLUGIN,
	DELAY_PLUGIN,
	FLANGER_PLUGIN
};

/* Audio info struct */
struct AudioInfo {
	uint32_t u32_in_ch_cnt;
	uint32_t u32_out_ch_cnt;
	double   f64_sampling_rate;
	uint32_t u32_buffer_size;
	uint32_t u32_mode;
	uint32_t u32_plugin_id;
	void     *p_plugin;
};

/* Plugin control funtions */
int filter_ctrl   (struct AudioInfo *p_audio_info);
int graphiceq_ctrl(struct AudioInfo *p_audio_info);
int delay_ctrl    (struct AudioInfo *p_audio_info);
int flanger_ctrl  (struct AudioInfo *p_audio_info);

#endif
