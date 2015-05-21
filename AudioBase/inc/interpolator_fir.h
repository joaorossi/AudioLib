/*
 * interpolator_fir.h
 *
 * Filtro interpolador para upsampling
 *
 *  Created on: Aug 7, 2014
 *      Author: joaorossi
 */

#ifndef INTERPOLATOR_FIR_H_
#define INTERPOLATOR_FIR_H_

#include "fixed_point.h"

/**
 * Estrutura de handle do interpolador fir
 */
typedef struct _interp_t
{

	q31_t pq31_coeffs;
	q31_t pq31_state;

	uint32_t u32_L;
	uint32_t u32_phaseLen;
	uint32_t u32_blockLen;

} interp_t;

/**
 * @brief	Inicializa a estrutura de handle do interpolador
 * @param	pitrp_interp	:	ponteiro para o handle do interpolador
 * @param	pq31_coeffs 	: 	ponteiro para o vetor de coeffs
 * @param	pq31_state  	:	ponteiro para o vetor de estados, o comprimento
 * 								do vetor é u32_blockLen + u32_phaseLen - 1
 * @param	u32_L			:	fator de oversampling
 * @param	u32_coeffsLen	:	comprimento do vetor de coeffs, deve ser
 * 								ser multiplo de u32_L
 * @param	u32_blockLen	:	comprimento do bloco de processemento
 * @return	1 caso a função seja mal sucedida
 */
int
interp_init(interp_t *pitrp_interp, q31_t *pq31_coeffs, q31_t *pq31_state, uint32_t u32_L, uint32_t u32_coeffsLen, uint32_t u32_blockLen);

/**
 * @brief	Retorna o handle do interpolador ao estado inicial
 * @param	pitrp_interp	:	ponteiro para o handle do interpolador
 * @return	1 caso a função seja mal sucedida
 */
int
interp_flush(interp_t *pitrp_interp);

/**
 * @brief	Realiza a interpolação sobre um vetor de amostras
 * @param	pitrp_interp	:	ponteiro para o handle do interpolador
 * @param	pq31_src		:	ponteiro para o vetor de amostras a serem processadas
 * @param	pq31_dst		:	ponteiro para o vetor de amostras processadas
 * @return	1 caso a função seja mal sucedida
 */
int
interp_do(interp_t *pitrp_interp, q31_t *pq31_src, q31_t pq31_dst);

#endif /* INTERPOLATOR_FIR_H_ */
