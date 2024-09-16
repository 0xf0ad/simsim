#include <assert.h>
#include <math.h>
#include <ginac/ginac.h>
#include <ginac/matrix.h>

inline void swap_rows(GiNaC::matrix& A, size_t a, size_t b){
	GiNaC::ex tmp;
	for(size_t i = 0; i < A.cols(); i++){
		tmp = A(a, i);
		A(a, 0) = A(b, 0);
		A(b, 0) = tmp;
	}
}

// guassian matrix elimmunation but spelled in frensh to make it more goofy
inline GiNaC::ex pivodgos(GiNaC::matrix& p_A, GiNaC::matrix& p_b){
	// a little optimization we know A is a square matrix (rows = cols)
	// else we r gonna freak out
	assert(p_A.rows() == p_A.cols());
	assert(p_b.cols() == 1);
	assert(p_b.rows() == p_A.rows());

	GiNaC::matrix A = p_A;
	GiNaC::matrix b = p_b;

	for(size_t i = 0; i < A.rows(); i++){

		if(A(i, i).is_zero()){
			for(size_t j = i + 1; j < A.rows(); j++){
				if(A(j, i) != 0){
					swap_rows(A, i, j);
					swap_rows(b, i, j);
					break;
				}
			}
		}

		for(size_t j = i + 1; j < A.rows(); j++){
			GiNaC::ex piv = A(j, i) / A(i, i);
			b(j, 0) -= b(i, 0) * piv;
			for(size_t k = i; k < A.cols(); k++)
				A(j, k) -= A(i, k) * piv;
		}
	}

	GiNaC::matrix x(A.rows(), 1); // Solution vector
	for (int i = A.rows() - 1; i >= 0; --i){
		GiNaC::ex sum = 0;
		for (size_t j = i + 1; j < A.cols(); ++j)
			sum += A(i, j) * x(j, 0);

		x(i, 0) = (b(i, 0) - sum) / A(i, i).normal();
	}

	return x;
}
