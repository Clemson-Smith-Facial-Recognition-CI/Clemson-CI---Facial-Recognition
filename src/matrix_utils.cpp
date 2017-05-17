/**
 * @file matrix_utils.cpp
 *
 * Library of helpful matrix functions.
 */
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "matrix_utils.h"

/**
 * Compute the COS distance between two column vectors.
 *
 * Cosine similarity is the cosine of the angle between x and y:
 * S_cos(x, y) = x * y / (||x|| * ||y||)
 *
 * Since S_cos is on [-1, 1], we transform S_cos to be on [0, 2]:
 * d_cos(x, y) = 1 - S_cos(x, y)
 *
 * @param A
 * @param i
 * @param B
 * @param j
 */
precision_t m_dist_COS(const Matrix& A, int i, const Matrix& B, int j)
{
	assert(A.rows == B.rows);
	assert(0 <= i && i < A.cols && 0 <= j && j < B.cols);

	// compute x * y
	precision_t x_dot_y = 0;

	int k;
	for ( k = 0; k < A.rows; k++ ) {
		x_dot_y += ELEM(A, k, i) * ELEM(B, k, j);
	}

	// compute ||x|| and ||y||
	precision_t abs_x = 0;
	precision_t abs_y = 0;

	for ( k = 0; k < A.rows; k++ ) {
		abs_x += ELEM(A, k, i) * ELEM(A, k, i);
		abs_y += ELEM(B, k, j) * ELEM(B, k, j);
	}

	// compute similarity
	precision_t similarity = x_dot_y / sqrtf(abs_x * abs_y);

	// compute scaled distance
	return 1 - similarity;
}

/**
 * Compute the L1 distance between two column vectors.
 *
 * L1 is the Taxicab distance:
 * d_L1(x, y) = |x - y|
 *
 * @param A
 * @param i
 * @param B
 * @param j
 */
precision_t m_dist_L1(const Matrix& A, int i, const Matrix& B, int j)
{
	assert(A.rows == B.rows);
	assert(0 <= i && i < A.cols && 0 <= j && j < B.cols);

	precision_t dist = 0;

	int k;
	for ( k = 0; k < A.rows; k++ ) {
		dist += fabsf(ELEM(A, k, i) - ELEM(B, k, j));
	}

	return dist;
}

/**
 * Compute the L2 distance between two column vectors.
 *
 * L2 is the Euclidean distance:
 * d_L2(x, y) = ||x - y||
 *
 * @param A
 * @param i
 * @param B
 * @param j
 */
precision_t m_dist_L2(const Matrix& A, int i, const Matrix& B, int j)
{
	assert(A.rows == B.rows);
	assert(0 <= i && i < A.cols && 0 <= j && j < B.cols);

	precision_t dist = 0;

	int k;
	for ( k = 0; k < A.rows; k++ ) {
		precision_t diff = ELEM(A, k, i) - ELEM(B, k, j);
		dist += diff * diff;
	}

	dist = sqrtf(dist);

	return dist;
}

/**
 * Copy a matrix X into a list X_c of class
 * submatrices.
 *
 * This function assumes that the columns in X
 * are grouped by class.
 *
 * @param X
 * @param y
 * @param c
 */
std::vector<Matrix> m_copy_classes(const Matrix& X, const std::vector<data_entry_t>& y, int c)
{
	std::vector<Matrix> X_c;

	int i, j;
	for ( i = 0, j = 0; i < c; i++ ) {
		int k = j;
		while ( k < X.cols && y[k].label == y[j].label ) {
			k++;
		}

		X_c.push_back(Matrix("X_c_i", X, j, k));
		j = k;
	}

	assert(j == X.cols);

	return X_c;
}

/**
 * Compute the mean of each class for a matrix X,
 * given by a list X_c of class submatrices.
 *
 * @param X_c
 * @param c
 */
std::vector<Matrix> m_class_means(const std::vector<Matrix>& X_c, int c)
{
	std::vector<Matrix> U;

	int i;
	for ( i = 0; i < c; i++ ) {
		U.push_back(X_c[i].mean_column("U_i"));
	}

	return U;
}

/**
 * Compute the class covariance matrices for a matrix X,
 * given by a list X_c of class submatrices.
 *
 * S_i = (X_c_i - U_i) * (X_c_i - U_i)'
 *
 * @param X_c
 * @param U
 * @param c
 */
std::vector<Matrix> m_class_scatters(const std::vector<Matrix>& X_c, const std::vector<Matrix>& U, int c)
{
	std::vector<Matrix> S;

	int i;
	for ( i = 0; i < c; i++ ) {
		Matrix X_c_i("X_c_i", X_c[i]);
		X_c_i.subtract_columns(U[i]);

		S.push_back(X_c_i.product("S_i", X_c_i, false, true));
	}

	return S;
}

/**
 * Compute the between-scatter matrix S_b for a matrix X,
 * given by a list X_c of class submatrices.
 *
 * S_b = sum(n_i * (u_i - u) * (u_i - u)', i=1:c),
 *
 * @param X_c
 * @param U
 * @param c
 */
Matrix m_scatter_between(const std::vector<Matrix>& X_c, const std::vector<Matrix>& U, int c)
{
	int N = U[0].rows;

	// compute the mean of all classes
	Matrix u("u", N, 1);

	int i;
	for ( i = 0; i < c; i++ ) {
		u.add(U[i]);
	}
	u.elem_mult(1.0f / c);

	// compute the between-scatter S_b
	Matrix S_b = Matrix::zeros("S_b", N, N);

	for ( i = 0; i < c; i++ ) {
		// compute S_b_i
		Matrix u_i("u_i - u", U[i]);
		u_i.subtract(u);

		Matrix S_b_i = u_i.product("S_b_i", u_i, false, true);
		S_b_i.elem_mult(X_c[i].cols);

		S_b.add(S_b_i);
	}

	return S_b;
}

/**
 * Compute the within-scatter matrix S_w for a matrix X,
 * given by a list X_c of class submatrices.
 *
 * S_w = sum((X_c_i - U_i) * (X_c_i - U_i)', i=1:c)
 *
 * @param X_c
 * @param U
 * @param c
 */
Matrix m_scatter_within(const std::vector<Matrix>& X_c, const std::vector<Matrix>& U, int c)
{
	// compute the within-scatter S_w
	int N = U[0].rows;
	Matrix S_w = Matrix::zeros("S_w", N, N);

	int i;
	for ( i = 0; i < c; i++ ) {
		// compute S_w_i
		Matrix X_c_i("X_c_i", X_c[i]);
		X_c_i.subtract_columns(U[i]);

		Matrix S_w_i = X_c_i.product("S_w_i", X_c_i, false, true);

		S_w.add(S_w_i);
	}

	return S_w;
}
