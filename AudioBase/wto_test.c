#include <stdio.h>
#include <fixedpoint.h>
#include <wave_table_osc.h>

#define WTO_TABLE_SIZE 64
#define BUFFER_SIZE    16
#define BUFFER_ITERATIONS 8

int
main(int argc, char *argv[])
{
	char   *str_err;
	void   *p_wto = NULL;
	q31_t  *pq31_buffer = NULL;
	int    i, j;
	double f64_proof;

	if (!(p_wto = wave_table_osc_init(WTO_TABLE_SIZE))) {
		fprintf(stderr, "Failed initializing WTO object.\n");
		goto error;
	}

	if (wave_table_osc_set_type(p_wto, 0)) {
		fprintf(stderr, "Failed setting table\n");
		goto error;
	}

	if (wave_table_osc_set_incr(p_wto, 0.5)) {
		fprintf(stderr, "Failed setting table inc\n");
		goto error;
	}

	if (!(pq31_buffer = calloc(1, BUFFER_SIZE * sizeof(q31_t)))) {
		fprintf(stderr, "Failed allocating output buffer.\n");
		goto error;
	}


	for (j = 0; j < BUFFER_ITERATIONS; j++) {
		if (wave_table_osc_do(p_wto, pq31_buffer, BUFFER_SIZE)) {
			fprintf(stderr, "Failed to process samples.\n");
			goto error;
		}
		for (i = 0; i < BUFFER_SIZE; i++) {
			f64_proof = Q31_TO_F64(pq31_buffer[i]);
			fprintf(stdout, "Sample #%d: %f\n", i, f64_proof);
		}
	}

	if (p_wto)
		wave_table_osc_deinit(p_wto);

	if (pq31_buffer)
		free(pq31_buffer);

	return 0;

error:

	if (p_wto)
		wave_table_osc_deinit(p_wto);

	if (pq31_buffer)
		free(pq31_buffer);

	return -1;
}
