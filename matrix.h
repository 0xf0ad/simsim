#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct{
	uint64_t size[2];
	double** entries;
} matrix_t;

inline void init_matrix(matrix_t* mat, uint64_t n, uint64_t m){
	mat->size[0] = n; mat->size[1] = m;
	mat->entries = (double**) malloc(sizeof(double*) * n);
	for (uint64_t i = 0; i < n; i++)
		mat->entries[i] = (double*) malloc(sizeof(double) * m);
}

inline void fill_zeros(matrix_t* mat){
	for(uint64_t i = 0; i < mat->size[0]; i++)
		for(uint64_t j = 0; j < mat->size[1]; j++)
			mat->entries[i][j] = 0;
}

inline void free_mat(matrix_t* mat){
	for (uint64_t i = 0; i < mat->size[0]; i++)
		free(mat->entries[i]);
	free(mat->entries);
	mat->size[0] = mat->size[1] = 0;
}

inline void mul_mat_scalar(matrix_t* mat, double scalar){
	for (uint64_t i = 0; i < mat->size[0]; i++)
		for (uint64_t j = 0; j < mat->size[1]; j++)
			mat->entries[i][j] *= scalar;
}

inline void mul_mat_mat(matrix_t* A, matrix_t* B, matrix_t* res){
	assert(A->size[1] == B->size[0]);
	init_matrix(res, A->size[0], B->size[1]);
	for (uint64_t i = 0; i < A->size[0]; i++){
		for (uint64_t j = 0; j < B->size[1]; j++){
			double product = 0.l;
			for (uint64_t k = 0; k < A->size[1]; k++)
				product += A->entries[i][k] * B->entries[k][j];
			res->entries[i][j] = product;
		}
	}
}

#endif //_MATRIX_H_