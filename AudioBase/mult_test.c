#include <fixedpoint.h>
#include <stdio.h>
#include <math.h>

/*#define LIN_INTERP(q31_x, q31_y, q31_frc)                                     \
({                                                                            \
	q31_t q31_frc_comp = 0x7ffffff - q31_frc;                                 \
	MUL__Q31(q31_x, q31_frc) + MUL__Q31(q31_y, q31_frc_comp);                 \
})*/

/*#define LIN_INTERP(q31_x, q31_y, q31_frc)            \
({                                                   \
	q31_t q31_frc_comp = 0x7ffffff - q31_frc;        \
	q31_t q31_frc_1 = MUL__Q31(q31_x, q31_frc);      \
	q31_t q31_frc_2 = MUL__Q31(q31_y, q31_frc_comp); \
	q31_frc_1 + q31_frc_2;                           \
})*/

/*#define LIN_INTERP(q31_x, q31_y, q31_frc) (MUL__Q31(q31_x, q31_frc) - MUL__Q31(q31_y, q31_frc) + q31_y)*/

int
main(void)
{
	q31_t q31_a, q31_b, q31_c, q31_d, q31_proof;
	double f64_a, f64_b, f64_c, f64_proof;

	f64_a = 0.02809823490;
/*	f64_b = -0.25;
	f64_c = 0.5;*/

	q31_a = F64_TO_Q31(f64_a);
/*	q31_b = F64_TO_Q31(f64_b);
	q31_c = F64_TO_Q31(f64_c);*/

	f64_proof = Q31_TO_F64(q31_a);

	printf("Original value: %lf\nProof result: %lf\n", f64_a, f64_proof);

	return 0;
}