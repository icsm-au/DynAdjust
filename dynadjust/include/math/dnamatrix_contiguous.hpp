//===========================================================================
// Name         : dnamatrix_contiguous.hpp
// Author       : Roger Fraser
// Contributors :
// Version      : 1.00
// Copyright    : Copyright 2017 Geoscience Australia
//
//                Licensed under the Apache License, Version 2.0 (the "License");
//                you may not use this file except in compliance with the License.
//                You may obtain a copy of the License at
//               
//                http ://www.apache.org/licenses/LICENSE-2.0
//               
//                Unless required by applicable law or agreed to in writing, software
//                distributed under the License is distributed on an "AS IS" BASIS,
//                WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//                See the License for the specific language governing permissions and
//                limitations under the License.
//
// Description  : DynAdjust Matrix library
//                Matrices are stored as a contiguous 1 dimensional array [row * column]
//                Storage buffer is ordered column wise to achieve highest efficiency with
//                Intel MKL
//============================================================================

#ifndef DNAMATRIX_CONTIGUOUS_H_
#define DNAMATRIX_CONTIGUOUS_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <mkl.h>

#include <exception>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <include/config/dnatypes.hpp>
#include <include/config/dnaversion.hpp>
#include <include/config/dnaconsts.hpp>
#include <include/exception/dnaexception.hpp>
#include <include/memory/dnamemory_handler.hpp>
#include <include/functions/dnatemplatecalcfuncs.hpp>

#ifdef _MSDEBUG
#include <include/ide/trace.hpp>
#endif

//#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind/bind.hpp>

#include <math.h>
#include <cstring>		// memset

using namespace std;
using namespace boost;
using namespace dynadjust::memory;
using namespace dynadjust::exception;

namespace dynadjust { namespace math {

class matrix_2d;
typedef vector<matrix_2d> v_mat_2d, *pv_mat_2d;
typedef v_mat_2d::iterator _it_v_mat_2d;
typedef v_mat_2d::const_iterator _it_v_mat_2d_const;
typedef vector<v_mat_2d> vv_mat_2d;

// Uncomment the following to store matrices in row-wise fashion
//#define DNAMATRIX_ROW_WISE

#if defined(DNAMATRIX_ROW_WISE)

// row * _mem_cols + column
#define DNAMATRIX_INDEX(no_rows, no_cols, row, column) row * no_cols + column

#else // DNAMATRIX_COL_WISE

// column * _mem_rows + row
#define DNAMATRIX_INDEX(no_rows, no_cols, row, column) column * no_rows + row

#endif

#define DNAMATRIX_ELEMENT(A, no_rows, no_cols, row, column) A[ DNAMATRIX_INDEX(no_rows, no_cols, row, column) ]


template <typename T>
std::size_t byteSize(const UINT32 elements=1)
{
	return elements * sizeof(T);
}

class matrix_2d: public new_handler_support<matrix_2d>
{
public:
	// Constructors/deconstructors
	matrix_2d();
	matrix_2d(const UINT32& rows, const UINT32& columns);		// explicit constructor
	matrix_2d(const UINT32& rows, const UINT32& columns, 
		const double data[], const UINT32& data_size, const UINT32& matrix_type = mtx_full);
	matrix_2d(const matrix_2d&);	// copy constructor
	~matrix_2d();					// destructor

	inline bool empty() { return _buffer == NULL; }

	std::size_t get_size();

	///////////////////////////////////////////////////////////////////////
	// Get
	inline UINT32 memRows() const { return _mem_rows; }
	inline UINT32 memColumns() const { return _mem_cols; }
	inline UINT32 rows() const { return _rows; }
	inline UINT32 columns() const { return _cols; }
	inline double* getbuffer() const { return _buffer; }
	
	// element retrieval
	// see DNAMATRIX_ROW_WISE
	inline double& get(const UINT32& row, const UINT32& column) const { return DNAMATRIX_ELEMENT(_buffer, _mem_rows, _mem_cols, row, column); }
	inline double* getbuffer(const UINT32& row, const UINT32& column) const { 
		return _buffer + DNAMATRIX_INDEX(_mem_rows, _mem_cols, row, column);
	}
	
	void submatrix(const UINT32& row_begin, const UINT32& col_begin, matrix_2d* dest, 
		const UINT32& rows, const UINT32& columns) const;
	matrix_2d submatrix(const UINT32& row_begin, const UINT32& col_begin, 
		const UINT32& rows, const UINT32& columns) const;
	
	inline double maxvalue() const { return get(_maxvalRow, _maxvalCol); }
	inline UINT32 maxvalueRow() const { return _maxvalRow; }
	inline UINT32 maxvalueCol() const { return _maxvalCol; }
	
	inline double* getelementref(const UINT32& row, const UINT32& column) const { return &(DNAMATRIX_ELEMENT(_buffer, _mem_rows, _mem_cols, row, column)); }
	inline double* getelementref(const UINT32& row, const UINT32& column) { return &(DNAMATRIX_ELEMENT(_buffer, _mem_rows, _mem_cols, row, column)); }
	
	inline void mem_rows(const UINT32& r) { _mem_rows = r; }
	inline void mem_columns(const UINT32& c) { _mem_cols = c; }
	inline void rows(const UINT32& r) { _rows = r; }
	inline void columns(const UINT32& c) { _cols = c; }
	inline void maxvalueRow(const UINT32& r) { _maxvalRow = r; }
	inline void maxvalueCol(const UINT32& c) { _maxvalCol = c; }
	
	inline void put(const UINT32& row, const UINT32& column, const double& value) { DNAMATRIX_ELEMENT(_buffer, _mem_rows, _mem_cols, row, column) = value; }
	
	inline UINT32 matrixType() const { return _matrixType; }
	inline void matrixType(const UINT32 t) { _matrixType = t; }
	
	// Matrix functions
	void copyelements(const UINT32& row_dest, const UINT32& column_dest, const matrix_2d& src, const UINT32& row_src, const UINT32& column_src, const UINT32& rows, const UINT32& columns);
	void copyelements(const UINT32& row_dest, const UINT32& column_dest, const matrix_2d* src, const UINT32& row_src, const UINT32& column_src, const UINT32& rows, const UINT32& columns);
	
	inline void elementadd(const UINT32& row, const UINT32& column, const double& increment) {
		*getelementref(row, column) += increment;
	}
	
	inline void elementsubtract(const UINT32& row, const UINT32& column, const double& decrement) {
		*getelementref(row, column) -= decrement;
	}

	inline void elementmultiply(const UINT32& row, const UINT32& column, const double& scale) {
		*getelementref(row, column) *= scale;
	}

#if defined(__SAFE_MATRIX_OPERATIONS__)
	//void copyelements(const UINT32& row_dest, const UINT32& column_dest, const UINT32& row_src, const UINT32& column_src, const UINT32& rows, const UINT32& columns);
	//void copyelements_safe(const UINT32& row_dest, const UINT32& column_dest, const UINT32& row_src, const UINT32& column_src, const UINT32& rows, const UINT32& columns);
	//void copyelements_safe(const UINT32& row_dest, const UINT32& column_dest, const matrix_2d& src, const UINT32& row_src, const UINT32& column_src, const UINT32& rows, const UINT32& columns);
	//double get_safe(const UINT32& row, const UINT32& column) const;
	//void put(const double data[], const UINT32& data_size);
	//void put_safe(const UINT32& row, const UINT32& column, const double& value);
	//void elementadd_safe(const UINT32& row, const UINT32& column, const double& increment);
	//void blockadd_safe(const UINT32& row_dest, const UINT32& col_dest, const matrix_2d& mat_src, 
	//					 const UINT32& row_src, const UINT32& col_src, const UINT32& rows, const UINT32& cols);
	//void blockTadd_safe(const UINT32& row_dest, const UINT32& col_dest, const matrix_2d& mat_src, 
	//					 const UINT32& row_src, const UINT32& col_src, const UINT32& rows, const UINT32& cols);
	//matrix_2d scale(const double& scalar,				//  ''
	//	const UINT32& row_begin, const UINT32& col_begin, 
	//	UINT32 rows=0, UINT32 columns=0);
	//void zero_safe(const UINT32& row_begin, const UINT32& col_begin, const UINT32& rows, const UINT32& columns);
#endif

	void blockadd(const UINT32& row_dest, const UINT32& col_dest, const matrix_2d& mat_src, 
						 const UINT32& row_src, const UINT32& col_src, const UINT32& rows, const UINT32& cols);
	void blockTadd(const UINT32& row_dest, const UINT32& col_dest, const matrix_2d& mat_src, 
						 const UINT32& row_src, const UINT32& col_src, const UINT32& rows, const UINT32& cols);
	void blocksubtract(const UINT32& row_dest, const UINT32& col_dest, const matrix_2d& mat_src, 
						 const UINT32& row_src, const UINT32& col_src, const UINT32& rows, const UINT32& cols);
	
	matrix_2d add(const matrix_2d& rhs);
	matrix_2d add(const matrix_2d& lhs, const matrix_2d& rhs);
	
	matrix_2d multiply(const matrix_2d& rhs);			// multiplication
	matrix_2d multiply(const matrix_2d& lhs, 
		const matrix_2d& rhs);							// multiplication

	matrix_2d multiply_mkl(const char* lhs_trans, 
		const matrix_2d& rhs, const char* rhs_trans);			// multiplication
	matrix_2d multiply_mkl(const matrix_2d& lhs, const char* lhs_trans, 
		const matrix_2d& rhs, const char* rhs_trans);			// multiplication

	matrix_2d multiply_square(const matrix_2d& lhs,		// multiplication, calculate upper triangle only, then copy to lower
		const matrix_2d& rhs);							
	matrix_2d multiply_square_triangular(const matrix_2d& lhs,		// multiplication, calculate upper triangle only, then copy to lower
		const matrix_2d& rhs);							
	matrix_2d multiply_square_t(const matrix_2d& lhs,	// same as multiply_square, except lhs is multiplied by transpose of rhs.
		const matrix_2d& rhs);							
	matrix_2d sweepinverse();							// Sweep inverse (good for rotation matrices)
	matrix_2d gaussianinverse();						// Gaussian inverse
	matrix_2d choleskyinverse(bool LOWER_IS_CLEARED=false);				// Cholesky inverse
	void decomposeupper();								// Cholesky decomposition 
	
	matrix_2d choleskyinverse_mkl(bool LOWER_IS_CLEARED=false);	// Cholesky inverse using MKL

	matrix_2d transpose(const matrix_2d&);				// Transpose
	matrix_2d transpose();								//  ''
	matrix_2d scale(const double& scalar);				// scale
	
	// overloaded operators
	// equality
	bool operator==(const matrix_2d& rhs) const {
		if (_mem_cols != _mem_cols)
			return false;
		if (_mem_rows != _mem_rows)
			return false;
		if (_cols != _cols)
			return false;
		if (_rows != _rows)
			return false;
		if (*_buffer != *_buffer)
			return false;
		if (_maxvalCol != _maxvalCol)
			return false;
		if (_maxvalRow != _maxvalRow)
			return false;
		if (_matrixType != _matrixType)
			return false;
		return true;
	}

	// equality
	bool operator!=(const matrix_2d& rhs) const {
		if (_mem_cols == _mem_cols)
			return false;
		if (_mem_rows == _mem_rows)
			return false;
		if (_cols == _cols)
			return false;
		if (_rows == _rows)
			return false;
		if (*_buffer == *_buffer)
			return false;
		if (_maxvalCol == _maxvalCol)
			return false;
		if (_maxvalRow == _maxvalRow)
			return false;
		if (_matrixType == _matrixType)
			return false;
		return true;
	}

	matrix_2d operator=(const matrix_2d& rhs);
	matrix_2d operator*(const double& rhs) const;
	matrix_2d operator*(const matrix_2d& rhs) const;
	matrix_2d operator+(const matrix_2d& rhs) const;
	matrix_2d operator-(const matrix_2d& rhs) const;
	
	matrix_2d operator*=(const matrix_2d& rhs);
	matrix_2d operator+=(const matrix_2d& rhs);
	matrix_2d operator-=(const matrix_2d& rhs);
	double operator() (const UINT32& row, const UINT32& column);

	// Initialisation / manipulation
	void allocate();
	void allocate(const UINT32& rows, const UINT32& columns);
	void setsize(const UINT32& rows, const UINT32& columns);		// sets matrix size to rows * columns only (buffer not allocated any memory)
	void redim(const UINT32& rows, const UINT32& columns);			// redimensions matrix to rows * columns
	void replace(const UINT32& rowstart, const UINT32& columnstart, const matrix_2d& newmat);
	void replace(const UINT32& rowstart, const UINT32& columnstart, const UINT32& rows, const UINT32& columns, const matrix_2d& newmat);
	//void append(const matrix_2d& newmat);
	//void appendcolumns(const UINT32& columns);
	//void appendrows(const UINT32& rows);
	void shrink(const UINT32& rows, const UINT32& columns);
	void grow(const UINT32& rows, const UINT32& columns);
	void clearlower();			// sets lower tri elements to zero
	void filllower();			// copies upper tri to lower
	void fillupper();			// copies lower tri to upper
	void zero();				// sets all elements to zero
	void zero(const UINT32& row_begin, const UINT32& col_begin, const UINT32& rows, const UINT32& columns);
	void identity();													// set matrix to Identity
	void identity(const UINT32& rowstart, const UINT32& columnstart);	// set sub-matrix to Identity (from
	void identity(const UINT32& rowstart, const UINT32& columnstart,	// at rowstart, columnstart to end)
		const UINT32& rows, const UINT32& columns);						// set sub-matrix to Identity

	// comparison
	void difference(const matrix_2d* lhs, const matrix_2d* rhs);
	void difference(const UINT32& row_begin, const UINT32& col_begin, const matrix_2d& lhs, const matrix_2d& rhs);
	void differenceabs(const matrix_2d& lhs, const matrix_2d& rhs);
	void differenceabs(const matrix_2d& lhs, const matrix_2d* rhs);
	double vectordifference(const matrix_2d& rhs);
	double compute_maximum_value();

	// Printing
	void coutmatrix(const string& sTitle, const short& precision) const;
	friend ostream& operator<< (ostream& os, const matrix_2d& rhs);
	friend ostream& operator<< (ostream& os, const matrix_2d* rhs);
	friend ostream& operator<< (ostream& os, const v_mat_2d& rhs);
	friend ostream& operator<< (ostream& os, const v_mat_2d* rhs);

	// Reading
	friend istream& operator>> (istream& is, matrix_2d& rhs);
	friend istream& operator>> (istream& is, matrix_2d* rhs);
	friend istream& operator>> (istream& is, v_mat_2d& rhs);
	friend istream& operator>> (istream& is, v_mat_2d* rhs);

	// Reading from memory mapped file
	void ReadMappedFileRegion(void* addr);

	// Writing to memory mapped file
	void WriteMappedFileRegion(void* addr);

	// debug
	void trace(const char* comment, const char* format) const;
	void trace(const char* comment, const char *submat_comment, const char* format,
		const UINT32& row_begin, const UINT32& col_begin, 
		const UINT32& rows, const UINT32& columns) const;

	//void replacebuffer(const UINT32& rows, const UINT32& columns, double**	buffer);

private:

	inline std::size_t buffersize() const { return _mem_rows * _mem_cols * sizeof(double); }

	void deallocate();
	void buy(const UINT32& rows, const UINT32& columns, double** mem_space);
	void copybuffer(const UINT32& rows, const UINT32& columns, const matrix_2d& oldmat);
	void copybuffer(const UINT32& rowstart, const UINT32& columnstart, 
		const UINT32& rows, const UINT32& columns, const matrix_2d& mat);
	//void copybuffer(const UINT32& rows, const UINT32& columns, double**	buffer);

	void sweep(UINT32 k1, UINT32 k2);		
	
	UINT32		_mem_cols;		// actual buffer size (cols)
	UINT32		_mem_rows;		// actual buffer size (rows)
	UINT32		_cols;		// number of actual cols
	UINT32		_rows;			// number of actual rows
	double*		_buffer;		// matrix buffer elements

	UINT32		_maxvalCol;		// col of max value
	UINT32		_maxvalRow;		// row of max value

	UINT32		_matrixType;	// full, upper/lower, sparse
};
	
}	// namespace math 
}	// namespace dynadjust 

#endif	// DNAMATRIX_CONTIGUOUS_H_

