//============================================================================
// Name         : bicubic_interpolation.hpp
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
// Description  : AusGeoid Grid File (NTv2) Interpolation library
//
//				  Interpolation routines from Numerical Recipes in C, The Art of
//				  Scientific Computing, Second Edition William H. Press, 
//				  Harvard-Smithsonian Center for Astrophysics, 
//				  Saul A. Teukolsky, Department of Physics, Cornell University
//				  William T. Vetterling, Polaroid Corporation
//				  Brian P. Flannery, EXXON Research and Engineering Company
//============================================================================

#ifndef DNAINTERPOLATIONFUNCS_H_
#define DNAINTERPOLATIONFUNCS_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <functional>
#include <math.h>
#include <typeinfo>

const int wt[16][16] =	{
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
	-3,0,0,3,0,0,0,0,-2,0,0,-1,0,0,0,0,
	2,0,0,-2,0,0,0,0,1,0,0,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
	0,0,0,0,-3,0,0,3,0,0,0,0,-2,0,0,-1,
	0,0,0,0,2,0,0,-2,0,0,0,0,1,0,0,1,
	-3,3,0,0,-2,-1,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,-3,3,0,0,-2,-1,0,0,
	9,-9,9,-9,6,3,-3,-6,6,-6,-3,3,4,2,1,2,
	-6,6,-6,6,-4,-2,2,4,-3,3,3,-3,-2,-1,-1,-2,
	2,-2,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,2,-2,0,0,1,1,0,0,
	-6,6,-6,6,-3,-3,3,3,-4,4,2,-2,-2,-2,-1,-1,
	4,-4,4,-4,2,2,-2,-2,2,-2,-2,2,1,1,1,1
};


// Given arrays y[1..4], y1[1..4], y2[1..4], and y12[1..4], containing the function, gradients,
// and cross derivative at the four grid points of a rectangular grid cell (numbered counterclockwise
// from the lower left), and given d1 and d2, the length of the grid cell in the 1- and
// 2-directions, this routine returns the table c[1..4][1..4] that is used by routine bcuint
// for bicubic interpolation.
template <class T>
void bcucof(T y[], T y1[], T y2[], T y12[], T c[4][4])
{
	int l(0), k(0), j(0), i(0);
	T xx(0.), cl[16], x[16];	

	for (i=0; i<4; i++)			// Pack a temporary vector x.
	{ 
		x[i] = y[i];
		x[i+4] = y1[i];
		x[i+8] = y2[i];
		x[i+12] = y12[i];
	}

	for (i=0; i<16; i++)		// Matrix multiply by the stored table.
	{
		xx = 0.0;
		for (k=0; k<16; k++) 
			xx += wt[i][k] * x[k];
		cl[i] = xx;
	}
	
	l = 0;

	for (i=0; i<4; i++)			// Unpack the result into the output table.
	{
		for (j=0; j<4; j++) 
			c[i][j] = cl[l++];
	}
}

// Bicubic interpolation within a grid square. Input quantities are y,y1,y2,y12 (as described in
// bcucof); x1l and x1u, the lower and upper coordinates of the grid square in the 1-direction;
// x2l and x2u likewise for the 2-direction; and x1,x2, the coordinates of the desired point for
// the interpolation. The interpolated function value is returned as ansy, and the interpolated
// gradient values as ansy1 and ansy2. This routine calls bcucof.

template <class T>
bool bcuint(T y[], T y1[], T y2[], T y12[], T& D1, T& D2, const T* longIncrement, const T* latIncrement, T *interpolant)//, T *ansy1, T *ansy2)
{
	//double double_type(1);
	//float float_type(1);

	//// test for floating point types
	//if ((typeid(D1) != typeid(double_type)) &&	// not a double
	//	(typeid(D1) != typeid(float_type)))		// and not a float... crash!
	//{
	//	throw runtime_error("Not a floating point type");
	//}

	int i;
	T t = D1 / *longIncrement;
	T u = D2 / *latIncrement;
	T c[4][4];
	
	bcucof(y, y1, y2, y12, c);
	
	*interpolant = 0.0;

	for (i=3; i>=0; i--)
		*interpolant = t * (*interpolant) + ((((c[i][3] * u) + c[i][2]) * u) + c[i][1]) * u + c[i][0];
	//	*ansy2= t * (*ansy2) + (3.0 * c[i][3] * u + 2.0 * c[i][2]) * u + c[i][1];
	//	*ansy1 = u * (*ansy1) + (3.0 * c[3][i] * t + 2.0 *  c[2][i]) * t + c[1][i];
	//}

	//*ansy1 /= D1;
	//*ansy2 /= D2;

	return true;
}

#endif