/*	Matrix.h

MIT License

Copyright (c) 2018 Fabian Herb

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <initializer_list>
#include <cassert>
#include <cmath>
#include <algorithm>

/// Generic matrix template class
template<int rows, int cols, typename T = float>
class Matrix
{
public:
	using ValueType = T;

	static const int kRows = rows;
	static const int kCols = cols;

	/// Default constructor
	Matrix() = default;

	/// Construct matrix from an array in row-major order
	Matrix(const T values[rows * cols])
	{
		for(int i = 0; i < rows; i++)
		{
			for(int j = 0; j < cols; j++)
				m[i][j] = values[i * cols + j];
		}
	}

	/// Construct matrix from two-dimensional array
	Matrix(const T values[rows][cols])
	{
		for(int i = 0; i < rows; i++)
		{
			for(int j = 0; j < cols; j++)
				m[i][j] = values[i][j];
		}
	}

	Matrix(std::initializer_list<T> init)
	{
		assert(init.size() == rows * cols);
		auto it = init.begin();
		for(int i = 0; i < rows; i++)
			for(int j = 0; j < cols; j++, ++it)
				m[i][j] = *it;
	}

	inline T* operator[](int row) {return m[row];}
	inline const T* operator[](int row) const {return m[row];}

	/** Eigen compatibility. */
	inline T operator()(int row, int col) const {return m[row][col];}

	/** Eigen compatibility. */
	inline T& operator()(int row, int col) {return m[row][col];}

	/// Matrix-matrix multiplication
	template<class M>
	Matrix<rows, M::kCols, T> operator*(const M& mat) const
	{
		static_assert(cols == M::kRows, "Columns of left operand must equal rows of right operand");
//		static_assert(rows == M::kCols);

		Matrix<rows, M::kCols, T> newMat;
		for(int r = 0; r < rows; r++)
		{
			for(int c = 0; c < M::kCols; c++)
			{
				T sum = 0;
				for(int k = 0; k < cols; ++k)
				{
					sum += m[r][k] * mat(k, c);
				}
				newMat(r, c) = sum;
			}
		}
		return newMat;
	}

	/// Matrix-scalar multiplication
	Matrix operator*(ValueType s) const
	{
		Matrix newMat;
		for(int r = 0; r < rows; r++)
		{
			for(int c = 0; c < cols; c++)
				newMat(r, c) = m[r][c] * s;
		}
		return newMat;
	}

	Matrix operator+(const Matrix& mat) const
	{
		Matrix newMat;
		for(int r = 0; r < rows; r++)
		{
			for(int c = 0; c < cols; c++)
				newMat(r, c) = m[r][c] + mat(r, c);
		}
		return newMat;
	}

	Matrix& operator+=(const Matrix& mat)
	{
		for(int r = 0; r < rows; r++)
			for(int c = 0; c < cols; c++)
				m[r][c] += mat(r, c);
		return *this;
	}

	Matrix& operator*=(const Matrix& mat)
	{
		*this = *this * mat;
		return *this;
	}

	Matrix& operator*=(ValueType s)
	{
		for(int r = 0; r < rows; r++)
			for(int c = 0; c < cols; c++)
				m[r][c] *= s;
		return *this;
	}

	Matrix& operator/=(ValueType s)
	{
		for(int r = 0; r < rows; r++)
			for(int c = 0; c < cols; c++)
				m[r][c] /= s;
		return *this;
	}

	static Matrix Identity()
	{
		Matrix newMat;
		for(int i = 0; i < rows; ++i)
		{
			for(int j = 0; j < cols; ++j)
			{
				if(i == j)
					newMat(i, j) = 1;
				else
					newMat(i, j) = 0;
			}
		}
		return newMat;
	}

protected:
	/** Row-major order. */
	T m[rows][cols];
};

/*****************************************************************************/

/// Scalar multiplication
template<int rows, int cols, typename T>
Matrix<rows, cols, T> operator*(T f, Matrix<rows, cols, T> m)
{
	Matrix<rows, cols, T> newMat;
	for(int r = 0; r < rows; ++r)
	{
		for(int c = 0; c < cols; ++c)
			newMat[r][c] = m[r][c] * f;
	}
	return newMat;
}

template<int rows, int cols, typename T>
std::ostream& operator<<(std::ostream& o, const Matrix<rows, cols, T>& m)
{
	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
			o << m[r][c] << " ";
		o << '\n';
	}
	return o;
}

#endif // MATRIX_H
