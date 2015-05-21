#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

/* Processing max values */
#define MAX_BUFFER_SIZE    4096
#define MAX_SAMPLING_RATE 96000

/* Fixed point types */
typedef int8_t  q7_t;
typedef int16_t q15_t;
typedef int32_t q31_t;
typedef int64_t q63_t;

/* Floating point types */
typedef float  f32_t;
typedef double f64_t;

/* Float to Fixed conversion consts */
#define FLT_Q63_CONV 0x8000000000000000
#define FLT_Q31_CONV 0x80000000
#define FLT_Q15_CONV 0x8000
#define FLT_Q7_CONV  0x80

/* Saturate double values exceeding Q31 boundaries */
#define SAT_ONE_F64(f64_X) f64_X >= 0.9999999997671 ? f64_X = 0.9999999997671 : f64_X <= -1.0 ? f64_X = -1.0 : f64_X

/* Conversion macros */
#define F32_TO_Q31(f32_X) ((q31_t)lrintf((FLT_Q31_CONV) * (f32_X))) /* TODO - Add saturation */
#define F64_TO_Q31(f64_X) ((q31_t)lrint ((FLT_Q31_CONV) * (SAT_ONE_F64(f64_X))))

#define Q31_TO_F64(q31_x) ((double)q31_x/(double)FLT_Q31_CONV)

/* Integer multiply */
#define MUL__Q31_Q31(q31_X, q31_Y) ((q63_t)q31_X * (q63_t)q31_Y)
#define MUL__Q31(q31_X, q31_Y) ((MUL__Q31_Q31(q31_X, q31_Y) >> 31))

/* Integer multiply accumulate */
#define MAC__Q63_Q31_Q31(q63_A, q31_X, q31_Y) (q63_A += MUL__Q31_Q31(q31_X, q31_Y))

#endif
